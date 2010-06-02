/* This file is part of Clementine.

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
#include "librarybackend.h"

#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QtDebug>
#include <QThread>
#include <QDateTime>
#include <QTimer>
#include <QSettings>

#include <taglib/fileref.h>
#include <taglib/tag.h>

QStringList LibraryWatcher::sValidImages;
QStringList LibraryWatcher::sValidPlaylists;

const char* LibraryWatcher::kSettingsGroup = "LibraryWatcher";

LibraryWatcher::LibraryWatcher(QObject* parent)
  : QObject(parent),
    stop_requested_(false),
    scan_on_startup_(true),
    rescan_timer_(new QTimer(this)),
    total_watches_(0)
{
  rescan_timer_->setInterval(1000);
  rescan_timer_->setSingleShot(true);

  if (sValidImages.isEmpty()) {
    sValidImages << "jpg" << "png" << "gif" << "jpeg";
    sValidPlaylists << "m3u" << "pls";
  }

  ReloadSettings();

  connect(rescan_timer_, SIGNAL(timeout()), SLOT(RescanPathsNow()));
}

LibraryWatcher::ScanTransaction::ScanTransaction(LibraryWatcher* watcher,
                                                 int dir, bool incremental)
  : dir_(dir),
    incremental_(incremental),
    watcher_(watcher),
    cached_songs_dirty_(true),
    known_subdirs_dirty_(true)
{
  emit watcher_->ScanStarted();
}

LibraryWatcher::ScanTransaction::~ScanTransaction() {
  // If we're stopping then don't commit the transaction
  if (watcher_->stop_requested_) return;

  if (!new_songs.isEmpty())
    emit watcher_->NewOrUpdatedSongs(new_songs);

  if (!touched_songs.isEmpty())
    emit watcher_->SongsMTimeUpdated(touched_songs);

  if (!deleted_songs.isEmpty())
    emit watcher_->SongsDeleted(deleted_songs);

  if (!new_subdirs.isEmpty())
    emit watcher_->SubdirsDiscovered(new_subdirs);

  if (!touched_subdirs.isEmpty())
    emit watcher_->SubdirsMTimeUpdated(touched_subdirs);

  emit watcher_->ScanFinished();

  // Watch the new subdirectories
  foreach (const Subdirectory& subdir, new_subdirs) {
    watcher_->AddWatch(watcher_->watched_dirs_[dir_].watcher, subdir.path);
  }
}

SongList LibraryWatcher::ScanTransaction::FindSongsInSubdirectory(const QString &path) {
  if (cached_songs_dirty_) {
    cached_songs_ = watcher_->backend_->FindSongsInDirectory(dir_);
    cached_songs_dirty_ = false;
  }

  // TODO: Make this faster
  SongList ret;
  foreach (const Song& song, cached_songs_) {
    if (song.filename().section('/', 0, -2) == path)
      ret << song;
  }
  return ret;
}

void LibraryWatcher::ScanTransaction::SetKnownSubdirs(const SubdirectoryList &subdirs) {
  known_subdirs_ = subdirs;
  known_subdirs_dirty_ = false;
}

bool LibraryWatcher::ScanTransaction::HasSeenSubdir(const QString &path) {
  if (known_subdirs_dirty_)
    SetKnownSubdirs(watcher_->backend_->SubdirsInDirectory(dir_));

  foreach (const Subdirectory& subdir, known_subdirs_) {
    if (subdir.path == path && subdir.mtime != 0)
      return true;
  }
  return false;
}

SubdirectoryList LibraryWatcher::ScanTransaction::GetImmediateSubdirs(const QString &path) {
  if (known_subdirs_dirty_)
    SetKnownSubdirs(watcher_->backend_->SubdirsInDirectory(dir_));

  SubdirectoryList ret;
  foreach (const Subdirectory& subdir, known_subdirs_) {
    if (subdir.path.left(subdir.path.lastIndexOf(QDir::separator())) == path &&
        subdir.mtime != 0) {
      ret << subdir;
    }
  }

  return ret;
}

SubdirectoryList LibraryWatcher::ScanTransaction::GetAllSubdirs() {
  if (known_subdirs_dirty_)
    SetKnownSubdirs(watcher_->backend_->SubdirsInDirectory(dir_));
  return known_subdirs_;
}

void LibraryWatcher::AddDirectory(const Directory& dir, const SubdirectoryList& subdirs) {
  DirData data;
  data.dir = dir;
  data.watcher = new QFileSystemWatcher(this);
  connect(data.watcher, SIGNAL(directoryChanged(QString)), SLOT(DirectoryChanged(QString)));
  watched_dirs_[dir.id] = data;

  if (subdirs.isEmpty()) {
    // This is a new directory that we've never seen before.
    // Scan it fully.
    ScanTransaction transaction(this, dir.id, false);
    transaction.SetKnownSubdirs(subdirs);
    ScanSubdirectory(dir.path, Subdirectory(), &transaction);
  } else {
    // We can do an incremental scan - looking at the mtimes of each
    // subdirectory and only rescan if the directory has changed.
    ScanTransaction transaction(this, dir.id, true);
    transaction.SetKnownSubdirs(subdirs);
    foreach (const Subdirectory& subdir, subdirs) {
      if (stop_requested_) return;

      if (scan_on_startup_)
        ScanSubdirectory(subdir.path, subdir, &transaction);

      AddWatch(data.watcher, subdir.path);
    }
  }

  emit CompilationsNeedUpdating();
}

void LibraryWatcher::ScanSubdirectory(
    const QString& path, const Subdirectory& subdir, ScanTransaction* t,
    bool force_noincremental) {
  QFileInfo path_info(path);
  if (!force_noincremental && t->is_incremental() &&
      subdir.mtime == path_info.lastModified().toTime_t()) {
    // The directory hasn't changed since last time
    return;
  }

  QMap<QString, QStringList> album_art;
  QStringList files_on_disk;
  SubdirectoryList my_new_subdirs;

  // If a directory is moved then only its parent gets a changed notification,
  // so we need to look and see if any of our children don't exist any more.
  // If one has been removed, "rescan" it to get the deleted songs
  SubdirectoryList previous_subdirs = t->GetImmediateSubdirs(path);
  foreach (const Subdirectory& subdir, previous_subdirs) {
    if (!QFile::exists(subdir.path) && subdir.path != path) {
      ScanSubdirectory(subdir.path, subdir, t, true);
    }
  }

  // First we "quickly" get a list of the files in the directory that we
  // think might be music.  While we're here, we also look for new subdirectories
  // and possible album artwork.
  QDirIterator it(path, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
  while (it.hasNext()) {
    if (stop_requested_) return;

    QString child(it.next());
    QFileInfo child_info(child);

    if (child_info.isDir()) {
      if (!t->HasSeenSubdir(child)) {
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
      else
        files_on_disk << child;
    }
  }

  if (stop_requested_) return;

  // Ask the database for a list of files in this directory
  SongList songs_in_db = t->FindSongsInSubdirectory(path);

  // Now compare the list from the database with the list of files on disk
  foreach (const QString& file, files_on_disk) {
    if (stop_requested_) return;

    Song matching_song;
    if (FindSongByPath(songs_in_db, file, &matching_song)) {
      // The song is in the database and still on disk.
      // Check the mtime to see if it's been changed since it was added.
      QFileInfo file_info(file);

      if (!file_info.exists()) {
        // Partially fixes race condition - if file was removed between being
        // added to the list and now.
        files_on_disk.removeAll(file);
        continue;
      }

      bool changed = matching_song.mtime() != file_info.lastModified().toTime_t();

      // Also want to look to see whether the album art has changed
      QString image = ImageForSong(file, album_art);
      if ((matching_song.art_automatic().isEmpty() && !image.isEmpty()) ||
          (!matching_song.art_automatic().isEmpty() && !QFile::exists(matching_song.art_automatic()))) {
        changed = true;
      }

      if (changed) {
        qDebug() << file << "changed";

        // It's changed - reread the metadata from the file
        Song song_on_disk;
        song_on_disk.InitFromFile(file, t->dir());
        if (!song_on_disk.is_valid())
          continue;
        song_on_disk.set_id(matching_song.id());
        song_on_disk.set_art_automatic(image);

        if (!matching_song.IsMetadataEqual(song_on_disk)) {
          qDebug() << file << "metadata changed";
          // Update the song in the DB
          t->new_songs << song_on_disk;
        } else {
          // Only the metadata changed
          t->touched_songs << song_on_disk;
        }
      }
    } else {
      // The song is on disk but not in the DB

      Song song;
      song.InitFromFile(file, t->dir());
      if (!song.is_valid())
        continue;
      qDebug() << file << "created";

      // Choose an image for the song
      song.set_art_automatic(ImageForSong(file, album_art));

      t->new_songs << song;
    }
  }

  // Look for deleted songs
  foreach (const Song& song, songs_in_db) {
    if (!files_on_disk.contains(song.filename())) {
      qDebug() << "Song deleted from disk:" << song.filename();
      t->deleted_songs << song;
    }
  }

  // Add this subdir to the new or touched list
  Subdirectory updated_subdir;
  updated_subdir.directory_id = t->dir();
  updated_subdir.mtime = path_info.exists() ?
                         path_info.lastModified().toTime_t() : 0;
  updated_subdir.path = path;

  if (subdir.directory_id == -1)
    t->new_subdirs << updated_subdir;
  else
    t->touched_subdirs << updated_subdir;

  // Recurse into the new subdirs that we found
  foreach (const Subdirectory& my_new_subdir, my_new_subdirs) {
    if (stop_requested_) return;
    ScanSubdirectory(my_new_subdir.path, my_new_subdir, t, true);
  }
}

void LibraryWatcher::AddWatch(QFileSystemWatcher* w, const QString& path) {
  if (!QFile::exists(path))
    return;

  w->addPath(path);
}

void LibraryWatcher::RemoveDirectory(const Directory& dir) {
  if (watched_dirs_.contains(dir.id)) {
    delete watched_dirs_[dir.id].watcher;
  }

  rescan_queue_.remove(dir.id);
  watched_dirs_.remove(dir.id);
}

bool LibraryWatcher::FindSongByPath(const SongList& list, const QString& path, Song* out) {
  // TODO: Make this faster
  foreach (const Song& song, list) {
    if (song.filename() == path) {
      *out = song;
      return true;
    }
  }
  return false;
}

void LibraryWatcher::DirectoryChanged(const QString &subdir) {
  // Find what dir it was in
  QFileSystemWatcher* watcher = qobject_cast<QFileSystemWatcher*>(sender());
  if (!watcher)
    return;

  Directory dir;
  foreach (const DirData& info, watched_dirs_) {
    if (info.watcher == watcher)
      dir = info.dir;
  }

  qDebug() << "Subdir" << subdir << "changed under directory" << dir.path << "id" << dir.id;

  // Queue the subdir for rescanning
  if (!rescan_queue_[dir.id].contains(subdir))
    rescan_queue_[dir.id] << subdir;

  rescan_timer_->start();
}

void LibraryWatcher::RescanPathsNow() {
  foreach (int dir, rescan_queue_.keys()) {
    if (stop_requested_) return;
    ScanTransaction transaction(this, dir, false);

    foreach (const QString& path, rescan_queue_[dir]) {
      if (stop_requested_) return;
      Subdirectory subdir;
      subdir.directory_id = dir;
      subdir.mtime = 0;
      subdir.path = path;
      ScanSubdirectory(path, subdir, &transaction);
    }
  }

  rescan_queue_.clear();

  emit CompilationsNeedUpdating();
}

QString LibraryWatcher::PickBestImage(const QStringList& images) {
  // This is used when there is more than one image in a directory.
  // Just pick the biggest image.

  int biggest_size = 0;
  QString biggest_path;

  foreach (const QString& path, images) {
    QImage image(path);
    if (image.isNull())
      continue;

    int size = image.width() * image.height();
    if (size > biggest_size) {
      biggest_size = size;
      biggest_path = path;
    }
  }

  return biggest_path;
}

QString LibraryWatcher::ImageForSong(const QString& path, QMap<QString, QStringList>& album_art) {
  QString dir(DirectoryPart(path));

  if (album_art.contains(dir)) {
    if (album_art[dir].count() == 1)
      return album_art[dir][0];
    else {
      QString best_image = PickBestImage(album_art[dir]);
      album_art[dir] = QStringList() << best_image;
      return best_image;
    }
  }
  return QString();
}

void LibraryWatcher::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  scan_on_startup_ = s.value("startup_scan", true).toBool();
}

void LibraryWatcher::IncrementalScanAsync() {
  QMetaObject::invokeMethod(this, "IncrementalScanNow", Qt::QueuedConnection);
}

void LibraryWatcher::IncrementalScanNow() {
  foreach (const DirData& data, watched_dirs_.values()) {
    ScanTransaction transaction(this, data.dir.id, true);
    SubdirectoryList subdirs(transaction.GetAllSubdirs());
    foreach (const Subdirectory& subdir, subdirs) {
      if (stop_requested_) return;

      ScanSubdirectory(subdir.path, subdir, &transaction);
    }
  }
  emit CompilationsNeedUpdating();
}
