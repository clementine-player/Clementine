/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "librarywatcher.h"

#include <fileref.h>
#include <tag.h>

#include <QDateTime>
#include <QDirIterator>
#include <QHash>
#include <QMutexLocker>
#include <QSet>
#include <QSettings>
#include <QThread>
#include <QTimer>
#include <QtDebug>

#include "core/filesystemwatcherinterface.h"
#include "core/logging.h"
#include "core/tagreaderclient.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "librarybackend.h"
#include "playlistparsers/cueparser.h"

// This is defined by one of the windows headers that is included by taglib.
#ifdef RemoveDirectory
#undef RemoveDirectory
#endif

namespace {
static const char* kNoMediaFile = ".nomedia";
static const char* kNoMusicFile = ".nomusic";
}  // namespace

static const int kUnfilteredImageLimit = 10;

QStringList LibraryWatcher::sValidImages;

const char* LibraryWatcher::kSettingsGroup = "LibraryWatcher";

LibraryWatcher::LibraryWatcher(QObject* parent)
    : QObject(parent),
      backend_(nullptr),
      task_manager_(nullptr),
      fs_watcher_(FileSystemWatcherInterface::Create(this)),
      scan_on_startup_(true),
      monitor_(true),
      rescan_timer_(new QTimer(this)),
      rescan_paused_(false),
      total_watches_(0),
      cue_parser_(new CueParser(backend_, this)) {
  rescan_timer_->setInterval(1000);
  rescan_timer_->setSingleShot(true);

  if (sValidImages.isEmpty()) {
    sValidImages << "jpg"
                 << "png"
                 << "gif"
                 << "jpeg";
  }

  ReloadSettings();

  connect(rescan_timer_, SIGNAL(timeout()), SLOT(RescanPathsNow()));
}

// Holding a reference to a directory is safe because a ScanTransaction object
// is only created on a stack and the removal of a directory from the watch
// list only occurs as a result of a signal and happens on the  watcher's
// thread. So the Directory object will not be deleted out from under us.
LibraryWatcher::ScanTransaction::ScanTransaction(
    LibraryWatcher* watcher, const LibraryWatcher::WatchedDir& dir,
    bool incremental, bool ignores_mtime)
    : progress_(0),
      progress_max_(0),
      dir_(dir),
      incremental_(incremental),
      ignores_mtime_(ignores_mtime),
      watcher_(watcher),
      cached_songs_dirty_(true),
      known_subdirs_dirty_(true) {
  QString description;
  if (watcher_->device_name_.isEmpty())
    description = tr("Updating library");
  else
    description = tr("Updating %1").arg(watcher_->device_name_);

  task_id_ = watcher_->task_manager_->StartTask(description);
  emit watcher_->ScanStarted(task_id_);
}

LibraryWatcher::ScanTransaction::~ScanTransaction() {
  // If we're stopping then don't commit the transaction
  if (aborted()) {
    watcher_->task_manager_->SetTaskFinished(task_id_);
    return;
  }

  if (!new_songs.isEmpty()) emit watcher_->NewOrUpdatedSongs(new_songs);

  if (!touched_songs.isEmpty()) emit watcher_->SongsMTimeUpdated(touched_songs);

  if (!deleted_songs.isEmpty()) emit watcher_->SongsDeleted(deleted_songs);

  if (!readded_songs.isEmpty()) emit watcher_->SongsReadded(readded_songs);

  if (!new_subdirs.isEmpty()) emit watcher_->SubdirsDiscovered(new_subdirs);

  if (!touched_subdirs.isEmpty())
    emit watcher_->SubdirsMTimeUpdated(touched_subdirs);

  watcher_->task_manager_->SetTaskFinished(task_id_);

  for (const Subdirectory& subdir : deleted_subdirs) {
    watcher_->RemoveWatch(dir_, subdir);
  }

  if (watcher_->monitor_) {
    // Watch the new subdirectories
    for (const Subdirectory& subdir : new_subdirs) {
      watcher_->AddWatch(dir_, subdir.path);
    }
  }
}

void LibraryWatcher::ScanTransaction::AddToProgress(int n) {
  progress_ += n;
  watcher_->task_manager_->SetTaskProgress(task_id_, progress_, progress_max_);
}

void LibraryWatcher::ScanTransaction::AddToProgressMax(int n) {
  progress_max_ += n;
  watcher_->task_manager_->SetTaskProgress(task_id_, progress_, progress_max_);
}

SongList LibraryWatcher::ScanTransaction::FindSongsInSubdirectory(
    const QString& path) {
  if (cached_songs_dirty_) {
    cached_songs_ = watcher_->backend_->FindSongsInDirectory(dir_id());
    cached_songs_dirty_ = false;
  }

  // TODO: Make this faster
  SongList ret;
  for (const Song& song : cached_songs_) {
    if (song.url().toLocalFile().section('/', 0, -2) == path) ret << song;
  }
  return ret;
}

void LibraryWatcher::ScanTransaction::SetKnownSubdirs(
    const SubdirectoryList& subdirs) {
  known_subdirs_ = subdirs;
  known_subdirs_dirty_ = false;
}

bool LibraryWatcher::ScanTransaction::HasSeenSubdir(const QString& path) {
  if (known_subdirs_dirty_)
    SetKnownSubdirs(watcher_->backend_->SubdirsInDirectory(dir_id()));

  for (const Subdirectory& subdir : known_subdirs_) {
    if (subdir.path == path && subdir.mtime != 0) return true;
  }
  return false;
}

SubdirectoryList LibraryWatcher::ScanTransaction::GetImmediateSubdirs(
    const QString& path) {
  if (known_subdirs_dirty_)
    SetKnownSubdirs(watcher_->backend_->SubdirsInDirectory(dir_id()));

  SubdirectoryList ret;
  for (const Subdirectory& subdir : known_subdirs_) {
    if (subdir.path.left(subdir.path.lastIndexOf(QDir::separator())) == path &&
        subdir.mtime != 0) {
      ret << subdir;
    }
  }

  return ret;
}

SubdirectoryList LibraryWatcher::ScanTransaction::GetAllSubdirs() {
  if (known_subdirs_dirty_)
    SetKnownSubdirs(watcher_->backend_->SubdirsInDirectory(dir_id()));
  return known_subdirs_;
}

void LibraryWatcher::WatchList::StopAll() {
  QMutexLocker l(&mutex_);
  for (WatchedDir& wdir : list_) {
    wdir.active_ = false;
  }
}

void LibraryWatcher::WatchList::Stop(int dir_id) {
  QMutexLocker l(&mutex_);
  if (list_.contains(dir_id)) {
    list_[dir_id].active_ = false;
  }
}

void LibraryWatcher::WatchList::Remove(int id) {
  QMutexLocker l(&mutex_);
  list_.remove(id);
}

void LibraryWatcher::WatchList::Add(const Directory& dir) {
  // Lock is not really necessary here, but it's also not going to block
  // anything, so include it for sanity and completeness.
  QMutexLocker l(&mutex_);
  list_[dir.id] = WatchedDir(dir);
}

void LibraryWatcher::AddDirectory(const Directory& dir,
                                  const SubdirectoryList& subdirs) {
  watched_dirs_.Add(dir);
  const WatchedDir& new_dir = watched_dirs_.list_[dir.id];

  if (subdirs.isEmpty()) {
    // This is a new directory that we've never seen before.
    // Scan it fully.
    ScanTransaction transaction(this, new_dir, false);
    transaction.SetKnownSubdirs(subdirs);
    transaction.AddToProgressMax(1);
    ScanSubdirectory(new_dir.path, Subdirectory(), &transaction);
  } else {
    // We can do an incremental scan - looking at the mtimes of each
    // subdirectory and only rescan if the directory has changed.
    ScanTransaction transaction(this, new_dir, true);
    transaction.SetKnownSubdirs(subdirs);
    transaction.AddToProgressMax(subdirs.count());
    for (const Subdirectory& subdir : subdirs) {
      if (transaction.aborted()) return;

      if (scan_on_startup_) ScanSubdirectory(subdir.path, subdir, &transaction);

      if (monitor_) AddWatch(new_dir, subdir.path);
    }
  }

  emit CompilationsNeedUpdating();
}

void LibraryWatcher::ScanSubdirectory(const QString& path,
                                      const Subdirectory& subdir,
                                      ScanTransaction* t,
                                      bool force_noincremental) {
  QFileInfo path_info(path);
  QDir path_dir(path);

  // Do not scan symlinked dirs that are already in collection
  if (path_info.isSymLink()) {
    QString real_path = path_info.symLinkTarget();
    for (const Directory& dir : watched_dirs_.list_) {
      if (real_path.startsWith(dir.path)) {
        t->AddToProgress(1);
        return;
      }
    }
  }

  // Do not scan directories containing a .nomedia or .nomusic file
  if (path_dir.exists(kNoMediaFile) || path_dir.exists(kNoMusicFile)) {
    t->AddToProgress(1);
    return;
  }

  if (!t->ignores_mtime() && !force_noincremental && t->is_incremental() &&
      subdir.mtime == path_info.lastModified().toTime_t()) {
    // The directory hasn't changed since last time
    t->AddToProgress(1);
    return;
  }

  QMap<QString, QStringList> album_art;
  QStringList files_on_disk;
  SubdirectoryList my_new_subdirs;

  // If a directory is moved then only its parent gets a changed notification,
  // so we need to look and see if any of our children don't exist any more.
  // If one has been removed, "rescan" it to get the deleted songs
  SubdirectoryList previous_subdirs = t->GetImmediateSubdirs(path);
  for (const Subdirectory& subdir : previous_subdirs) {
    if (!QFile::exists(subdir.path) && subdir.path != path) {
      t->AddToProgressMax(1);
      ScanSubdirectory(subdir.path, subdir, t, true);
    }
  }

  // First we "quickly" get a list of the files in the directory that we
  // think might be music.  While we're here, we also look for new
  // subdirectories
  // and possible album artwork.
  QDirIterator it(
      path, QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
  while (it.hasNext()) {
    if (t->aborted()) return;

    QString child(it.next());
    QFileInfo child_info(child);

    if (child_info.isDir()) {
      if (!child_info.isHidden() && !t->HasSeenSubdir(child)) {
        // We haven't seen this subdirectory before - add it to a list and
        // later we'll tell the backend about it and scan it.
        Subdirectory new_subdir;
        new_subdir.directory_id = -1;
        new_subdir.path = child;
        new_subdir.mtime = child_info.lastModified().toTime_t();
        my_new_subdirs << new_subdir;
      }
    } else {
      QString ext_part(ExtensionPart(child));
      QString dir_part(DirectoryPart(child));

      if (sValidImages.contains(ext_part))
        album_art[dir_part] << child;
      else if (!child_info.isHidden())
        files_on_disk << child;
    }
  }

  if (t->aborted()) return;

  // Ask the database for a list of files in this directory
  SongList songs_in_db = t->FindSongsInSubdirectory(path);

  QSet<QString> cues_processed;

  // Now compare the list from the database with the list of files on disk
  for (const QString& file : files_on_disk) {
    if (t->aborted()) return;

    // associated cue
    QString matching_cue = NoExtensionPart(file) + ".cue";

    Song matching_song;
    if (FindSongByPath(songs_in_db, file, &matching_song)) {
      uint matching_cue_mtime = GetMtimeForCue(matching_cue);

      // The song is in the database and still on disk.
      // Check the mtime to see if it's been changed since it was added.
      QFileInfo file_info(file);

      if (!file_info.exists()) {
        // Partially fixes race condition - if file was removed between being
        // added to the list and now.
        files_on_disk.removeAll(file);
        continue;
      }

      // cue sheet's path from library (if any)
      QString song_cue = matching_song.cue_path();
      uint song_cue_mtime = GetMtimeForCue(song_cue);

      bool cue_deleted = song_cue_mtime == 0 && matching_song.has_cue();
      bool cue_added = matching_cue_mtime != 0 && !matching_song.has_cue();

      // watch out for cue songs which have their mtime equal to
      // qMax(media_file_mtime, cue_sheet_mtime)
      bool changed =
          (matching_song.mtime() !=
           qMax(file_info.lastModified().toTime_t(), song_cue_mtime)) ||
          cue_deleted || cue_added;

      // Also want to look to see whether the album art has changed
      QString image = ImageForSong(file, &album_art, t);
      if ((matching_song.art_automatic().isEmpty() && !image.isEmpty()) ||
          (!matching_song.art_automatic().isEmpty() &&
           !matching_song.has_embedded_cover() &&
           !QFile::exists(matching_song.art_automatic()))) {
        changed = true;
      }

      // the song's changed - reread the metadata from file
      if (t->ignores_mtime() || changed) {
        qLog(Debug) << file << "changed";

        // if cue associated...
        if (!cue_deleted && (matching_song.has_cue() || cue_added)) {
          UpdateCueAssociatedSongs(file, path, matching_cue, image, t);
          // if no cue or it's about to lose it...
        } else {
          UpdateNonCueAssociatedSong(file, matching_song, image, cue_deleted,
                                     t);
        }
      }

      // nothing has changed - mark the song available without re-scanning
      if (matching_song.is_unavailable()) t->readded_songs << matching_song;

    } else {
      // The song is on disk but not in the DB
      SongList song_list =
          ScanNewFile(file, path, matching_cue, &cues_processed);

      if (song_list.isEmpty()) {
        continue;
      }

      qLog(Debug) << file << "created";
      // choose an image for the song(s)
      QString image = ImageForSong(file, &album_art, t);

      for (Song song : song_list) {
        song.set_directory_id(t->dir_id());
        if (song.art_automatic().isEmpty()) song.set_art_automatic(image);

        t->new_songs << song;
      }
    }
  }

  // Look for deleted songs
  for (const Song& song : songs_in_db) {
    if (!song.is_unavailable() &&
        !files_on_disk.contains(song.url().toLocalFile())) {
      qLog(Debug) << "Song deleted from disk:" << song.url().toLocalFile();
      t->deleted_songs << song;
    }
  }

  // Add this subdir to the new or touched list
  Subdirectory updated_subdir;
  updated_subdir.directory_id = t->dir_id();
  updated_subdir.mtime =
      path_info.exists() ? path_info.lastModified().toTime_t() : 0;
  updated_subdir.path = path;

  if (subdir.directory_id == -1)
    t->new_subdirs << updated_subdir;
  else
    t->touched_subdirs << updated_subdir;

  if (updated_subdir.mtime ==
      0) {  // Subdirectory deleted, mark it for removal from the watcher.
    t->deleted_subdirs << updated_subdir;
  }

  t->AddToProgress(1);

  // Recurse into the new subdirs that we found
  t->AddToProgressMax(my_new_subdirs.count());
  for (const Subdirectory& my_new_subdir : my_new_subdirs) {
    if (t->aborted()) return;
    ScanSubdirectory(my_new_subdir.path, my_new_subdir, t, true);
  }
}

void LibraryWatcher::UpdateCueAssociatedSongs(const QString& file,
                                              const QString& path,
                                              const QString& matching_cue,
                                              const QString& image,
                                              ScanTransaction* t) {
  QFile cue(matching_cue);
  cue.open(QIODevice::ReadOnly);

  SongList old_sections = backend_->GetSongsByUrl(QUrl::fromLocalFile(file));

  QHash<quint64, Song> sections_map;
  for (const Song& song : old_sections) {
    sections_map[song.beginning_nanosec()] = song;
  }

  QSet<int> used_ids;

  // update every song that's in the cue and library
  for (Song cue_song : cue_parser_->Load(&cue, matching_cue, path)) {
    cue_song.set_directory_id(t->dir_id());

    Song matching = sections_map[cue_song.beginning_nanosec()];
    // a new section
    if (!matching.is_valid()) {
      t->new_songs << cue_song;
      // changed section
    } else {
      PreserveUserSetData(file, image, matching, &cue_song, t);
      used_ids.insert(matching.id());
    }
  }

  // sections that are now missing
  for (const Song& matching : old_sections) {
    if (!used_ids.contains(matching.id())) {
      t->deleted_songs << matching;
    }
  }
}

void LibraryWatcher::UpdateNonCueAssociatedSong(const QString& file,
                                                const Song& matching_song,
                                                const QString& image,
                                                bool cue_deleted,
                                                ScanTransaction* t) {
  // if a cue got deleted, we turn it's first section into the new
  // 'raw' (cueless) song and we just remove the rest of the sections
  // from the library
  if (cue_deleted) {
    for (const Song& song :
         backend_->GetSongsByUrl(QUrl::fromLocalFile(file))) {
      if (!song.IsMetadataEqual(matching_song)) {
        t->deleted_songs << song;
      }
    }
  }

  Song song_on_disk;
  song_on_disk.set_directory_id(t->dir_id());
  TagReaderClient::Instance()->ReadFileBlocking(file, &song_on_disk);

  if (song_on_disk.is_valid()) {
    PreserveUserSetData(file, image, matching_song, &song_on_disk, t);
  }
}

SongList LibraryWatcher::ScanNewFile(const QString& file, const QString& path,
                                     const QString& matching_cue,
                                     QSet<QString>* cues_processed) {
  SongList song_list;

  uint matching_cue_mtime = GetMtimeForCue(matching_cue);
  // if it's a cue - create virtual tracks
  if (matching_cue_mtime) {
    // don't process the same cue many times
    if (cues_processed->contains(matching_cue)) return song_list;

    QFile cue(matching_cue);
    cue.open(QIODevice::ReadOnly);

    // Ignore FILEs pointing to other media files. Also, watch out for incorrect
    // media files. Playlist parser for CUEs considers every entry in sheet
    // valid and we don't want invalid media getting into library!
    QString file_nfd = file.normalized(QString::NormalizationForm_D);
    for (const Song& cue_song : cue_parser_->Load(&cue, matching_cue, path)) {
      if (cue_song.url().toLocalFile().normalized(
              QString::NormalizationForm_D) == file_nfd) {
        if (TagReaderClient::Instance()->IsMediaFileBlocking(file)) {
          song_list << cue_song;
        }
      }
    }

    if (!song_list.isEmpty()) {
      *cues_processed << matching_cue;
    }

    // it's a normal media file
  } else {
    Song song;
    TagReaderClient::Instance()->ReadFileBlocking(file, &song);

    if (song.is_valid()) {
      song_list << song;
    }
  }

  return song_list;
}

void LibraryWatcher::PreserveUserSetData(const QString& file,
                                         const QString& image,
                                         const Song& matching_song, Song* out,
                                         ScanTransaction* t) {
  out->set_id(matching_song.id());

  // Previous versions of Clementine incorrectly overwrote this and
  // stored it in the DB, so we can't rely on matching_song to
  // know if it has embedded artwork or not, but we can check here.
  if (!out->has_embedded_cover()) out->set_art_automatic(image);

  out->MergeUserSetData(matching_song);

  // The song was deleted from the database (e.g. due to an unmounted
  // filesystem), but has been restored.
  if (matching_song.is_unavailable()) {
    qLog(Debug) << file << " unavailable song restored";

    t->new_songs << *out;
  } else if (!matching_song.IsMetadataEqual(*out)) {
    qLog(Debug) << file << "metadata changed";

    // Update the song in the DB
    t->new_songs << *out;
  } else {
    // Only the mtime's changed
    t->touched_songs << *out;
  }
}

uint LibraryWatcher::GetMtimeForCue(const QString& cue_path) {
  // slight optimisation
  if (cue_path.isEmpty()) {
    return 0;
  }

  const QFileInfo file_info(cue_path);
  if (!file_info.exists()) {
    return 0;
  }

  const QDateTime cue_last_modified = file_info.lastModified();

  return cue_last_modified.isValid() ? cue_last_modified.toTime_t() : 0;
}

void LibraryWatcher::AddWatch(const Directory& dir, const QString& path) {
  QFileInfo info(path);
  if (!info.exists() || !info.isReadable()) return;

  connect(fs_watcher_, SIGNAL(PathChanged(const QString&)), this,
          SLOT(DirectoryChanged(const QString&)), Qt::UniqueConnection);
  if (!fs_watcher_->AddPath(path)) {
    // Since this may be a system error, don't spam the user.
    static int errCount = 0;
    if (errCount++ == 0) {
#ifdef Q_OS_LINUX
      // The Linux implementation of QFileSystemWatcher utilizes inotify, so
      // the limit in /proc/sys/fs/inotify/max_user_watches may be a problem
      // in large libraries.
      const char* fmt =
          "Failed to watch %1\n"
          "On a Linux system, this may be due to the inotify max_user_watches "
          "limit.\n\n"
          "This error will not be shown again during this session.";
#else
      const char* fmt =
          "Failed to watch %1 for unknown reasons.\n\n"
          "This error will not be shown again during this session.";
#endif
      emit Error(tr(fmt).arg(path));
    }
    return;
  }

  subdir_mapping_[path] = dir;
}

void LibraryWatcher::RemoveWatch(const Directory& dir,
                                 const Subdirectory& subdir) {
  for (const QString& subdir_path : subdir_mapping_.keys(dir)) {
    if (subdir_path != subdir.path) continue;
    fs_watcher_->RemovePath(subdir_path);
    subdir_mapping_.remove(subdir_path);
    break;
  }
}

void LibraryWatcher::RemoveDirectory(int dir_id) {
  watched_dirs_.Stop(dir_id);
  // Invoke the DoRemoveDirectory slot on the watcher's thread.
  QMetaObject::invokeMethod(this, "DoRemoveDirectory", Q_ARG(int, dir_id));
}

void LibraryWatcher::DoRemoveDirectory(int dir_id) {
  rescan_queue_.remove(dir_id);

  const WatchedDir& dir = watched_dirs_.list_[dir_id];
  // Stop watching the directory's subdirectories
  for (const QString& subdir_path : subdir_mapping_.keys(dir)) {
    fs_watcher_->RemovePath(subdir_path);
    subdir_mapping_.remove(subdir_path);
  }
  watched_dirs_.Remove(dir_id);
}

bool LibraryWatcher::FindSongByPath(const SongList& list, const QString& path,
                                    Song* out) {
  // TODO: Make this faster
  for (const Song& song : list) {
    if (song.url().toLocalFile() == path) {
      *out = song;
      return true;
    }
  }
  return false;
}

void LibraryWatcher::DirectoryChanged(const QString& subdir) {
  // Find what dir it was in
  QHash<QString, Directory>::const_iterator it =
      subdir_mapping_.constFind(subdir);
  if (it == subdir_mapping_.constEnd()) {
    return;
  }
  Directory dir = *it;

  qLog(Debug) << "Subdir" << subdir << "changed under directory" << dir.path
              << "id" << dir.id;

  // Queue the subdir for rescanning
  if (!rescan_queue_[dir.id].contains(subdir)) rescan_queue_[dir.id] << subdir;

  if (!rescan_paused_) rescan_timer_->start();
}

void LibraryWatcher::RescanPathsNow() {
  for (int id : rescan_queue_.keys()) {
    if (!watched_dirs_.list_.contains(id)) {
      qLog(Warning) << "Rescan id" << id << "not in watch list.";
      continue;
    }
    const WatchedDir& dir = watched_dirs_.list_[id];

    if (!dir.active_) continue;

    ScanTransaction transaction(this, dir, false);
    transaction.AddToProgressMax(rescan_queue_[id].count());

    for (const QString& path : rescan_queue_[id]) {
      if (transaction.aborted()) return;
      Subdirectory subdir;
      subdir.directory_id = id;
      subdir.mtime = 0;
      subdir.path = path;
      ScanSubdirectory(path, subdir, &transaction);
    }
  }

  rescan_queue_.clear();

  emit CompilationsNeedUpdating();
}

QString LibraryWatcher::PickBestImage(const QStringList& images,
                                      ScanTransaction* t) {
  // This is used when there is more than one image in a directory.
  // Pick the biggest image that matches the most important filter

  QStringList filtered;

  for (const QString& filter_text : best_image_filters_) {
    // the images in the images list are represented by a full path,
    // so we need to isolate just the filename
    for (const QString& image : images) {
      QFileInfo file_info(image);
      QString filename(file_info.fileName());
      if (filename.contains(filter_text, Qt::CaseInsensitive))
        filtered << image;
    }

    /* We assume the filters are give in the order best to worst, so
      if we've got a result, we go with it. Otherwise we might
      start capturing more generic rules */
    if (!filtered.isEmpty()) break;
  }

  if (filtered.isEmpty()) {
    // If we're scanning a device, we may hit a directory that contains
    // multiple types of media. An example is a camera directory on a smart
    // phone that contains JPG and MP4. We don't want to cycle through hundreds
    // of images for each audio file found, so we've set a threshold to try to
    // detect this case.
    if (images.count() > kUnfilteredImageLimit) {
      return "";
    }

    // the filter was too restrictive, just use the original list
    filtered = images;
  }

  int biggest_size = 0;
  QString biggest_path;

  for (const QString& path : filtered) {
    if (t->aborted()) return "";

    QImage image(path);
    if (image.isNull()) continue;

    int size = image.width() * image.height();
    if (size > biggest_size) {
      biggest_size = size;
      biggest_path = path;
    }
  }

  return biggest_path;
}

QString LibraryWatcher::ImageForSong(const QString& path,
                                     QMap<QString, QStringList>* album_art,
                                     ScanTransaction* t) {
  QString dir(DirectoryPart(path));

  if (album_art->contains(dir)) {
    if (album_art->value(dir).count() == 1)
      return album_art->value(dir)[0];
    else {
      QString best_image = PickBestImage(album_art->value(dir), t);
      album_art->insert(dir, QStringList() << best_image);
      return best_image;
    }
  }
  return QString();
}

void LibraryWatcher::ReloadSettingsAsync() {
  QMetaObject::invokeMethod(this, "ReloadSettings", Qt::QueuedConnection);
}

void LibraryWatcher::ReloadSettings() {
  const bool was_monitoring_before = monitor_;

  QSettings s;
  s.beginGroup(kSettingsGroup);
  scan_on_startup_ = s.value("startup_scan", true).toBool();
  monitor_ = s.value("monitor", true).toBool();

  best_image_filters_.clear();
  QStringList filters = s.value("cover_art_patterns", QStringList() << "front"
                                                                    << "cover")
                            .toStringList();
  for (const QString& filter : filters) {
    QString s = filter.trimmed();
    if (!s.isEmpty()) best_image_filters_ << s;
  }

  if (!monitor_ && was_monitoring_before) {
    fs_watcher_->Clear();
  } else if (monitor_ && !was_monitoring_before) {
    // Add all directories to all QFileSystemWatchers again
    for (const Directory& dir : watched_dirs_.list_.values()) {
      SubdirectoryList subdirs = backend_->SubdirsInDirectory(dir.id);
      for (const Subdirectory& subdir : subdirs) {
        AddWatch(dir, subdir.path);
      }
    }
  }
}

void LibraryWatcher::SetRescanPausedAsync(bool pause) {
  QMetaObject::invokeMethod(this, "SetRescanPaused", Qt::QueuedConnection,
                            Q_ARG(bool, pause));
}

void LibraryWatcher::SetRescanPaused(bool pause) {
  rescan_paused_ = pause;
  if (!rescan_paused_ && !rescan_queue_.isEmpty()) RescanPathsNow();
}

void LibraryWatcher::IncrementalScanAsync() {
  QMetaObject::invokeMethod(this, "IncrementalScanNow", Qt::QueuedConnection);
}

void LibraryWatcher::FullScanAsync() {
  QMetaObject::invokeMethod(this, "FullScanNow", Qt::QueuedConnection);
}

void LibraryWatcher::IncrementalScanNow() { PerformScan(true, false); }

void LibraryWatcher::FullScanNow() { PerformScan(false, true); }

void LibraryWatcher::PerformScan(bool incremental, bool ignore_mtimes) {
  for (const WatchedDir& dir : watched_dirs_.list_.values()) {
    ScanTransaction transaction(this, dir, incremental, ignore_mtimes);
    SubdirectoryList subdirs(transaction.GetAllSubdirs());
    transaction.AddToProgressMax(subdirs.count());

    for (const Subdirectory& subdir : subdirs) {
      if (transaction.aborted()) return;

      ScanSubdirectory(subdir.path, subdir, &transaction);
    }
  }

  emit CompilationsNeedUpdating();
}
