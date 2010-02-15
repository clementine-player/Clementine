#include "librarywatcher.h"
#include "librarybackend.h"
#include "enginebase.h"

#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QtDebug>
#include <QThread>
#include <QDateTime>
#include <QTimer>

#include <taglib/fileref.h>
#include <taglib/tag.h>


LibraryWatcher::LibraryWatcher(QObject* parent)
  : QObject(parent),
    fs_watcher_(new QFileSystemWatcher(this)),
    rescan_timer_(new QTimer(this)),
    total_watches_(0)
{
  rescan_timer_->setInterval(1000);
  rescan_timer_->setSingleShot(true);

  connect(fs_watcher_, SIGNAL(directoryChanged(QString)), SLOT(DirectoryChanged(QString)));
  connect(rescan_timer_, SIGNAL(timeout()), SLOT(RescanPathsNow()));
}

void LibraryWatcher::AddDirectories(const DirectoryList& directories) {
  // Iterate through each directory to find a list of files that look like they
  // could be music.

  foreach (const Directory& dir, directories) {
    paths_watched_[dir.path] = dir;
    ScanDirectory(dir.path);

    // Start monitoring this directory for more changes
    fs_watcher_->addPath(dir.path);
    ++total_watches_;

    // And all the subdirectories
    QDirIterator it(dir.path,
                    QDir::NoDotAndDotDot | QDir::Dirs,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QString subdir(it.next());
      fs_watcher_->addPath(subdir);
      paths_watched_[subdir] = dir;
    #ifdef Q_OS_DARWIN
    if (++total_watches_ > kMaxWatches) {
      qWarning() << "Trying to watch more files than we can manage";
      return;
    }
    #endif
    }
  }
}

void LibraryWatcher::RemoveDirectories(const DirectoryList &directories) {
  foreach (const Directory& dir, directories) {
    fs_watcher_->removePath(dir.path);
    paths_watched_.remove(dir.path);
    paths_needing_rescan_.removeAll(dir.path);

    // And all the subdirectories
    QDirIterator it(dir.path,
                    QDir::NoDotAndDotDot | QDir::Dirs,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QString subdir(it.next());
      fs_watcher_->removePath(subdir);
      paths_watched_.remove(subdir);
    }
  }
}

void LibraryWatcher::ScanDirectory(const QString& path) {
  const Directory& dir = paths_watched_[path];
  qDebug() << "Scanning" << path;
  emit ScanStarted();

  QStringList files_on_disk;
  QDirIterator it(dir.path,
                  QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                  QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
  while (it.hasNext()) {
    QString path(it.next());

    // Don't bother if the engine can't decode it
    if (!engine_->canDecode(QUrl::fromLocalFile(path)))
      continue;

    files_on_disk << path;
  }

  // Ask the database for a list of files in this directory
  SongList songs_in_db = backend_->FindSongsInDirectory(dir.id);

  // Now compare the list from the database with the list of files on disk
  SongList new_songs;
  SongList touched_songs;
  foreach (const QString& file, files_on_disk) {
    Song matching_song;
    if (FindSongByPath(songs_in_db, file, &matching_song)) {
      // The song is in the database and still on disk.
      // Check the mtime to see if it's been changed since it was added.

      if (matching_song.mtime() != QFileInfo(file).lastModified().toTime_t()) {
        qDebug() << file << "changed";

        // It's changed - reread the metadata from the file
        Song song_on_disk;
        song_on_disk.InitFromFile(file, dir.id);
        song_on_disk.set_id(matching_song.id());

        if (!matching_song.IsMetadataEqual(song_on_disk)) {
          qDebug() << file << "metadata changed";
          // Update the song in the DB
          new_songs << song_on_disk;
        } else {
          // Only the metadata changed
          touched_songs << song_on_disk;
        }
      }
    } else {
      // The song is on disk but not in the DB

      Song song;
      song.InitFromFile(file, dir.id);
      if (!song.is_valid())
        continue;
      qDebug() << file << "created";

      new_songs << song;
    }
  }

  if (!new_songs.isEmpty())
    emit NewOrUpdatedSongs(new_songs);

  if (!touched_songs.isEmpty())
    emit SongsMTimeUpdated(touched_songs);

  // Look for deleted songs
  SongList deleted_songs;
  foreach (const Song& song, songs_in_db) {
    if (!files_on_disk.contains(song.filename())) {
      qDebug() << "Song deleted from disk:" << song.filename();
      deleted_songs << song;
    }
  }

  if (!deleted_songs.isEmpty())
    emit SongsDeleted(deleted_songs);

  qDebug() << "Finished scanning" << path;
  emit ScanFinished();
}

bool LibraryWatcher::FindSongByPath(const SongList& list, const QString& path, Song* out) {
  foreach (const Song& song, list) {
    if (song.filename() == path) {
      *out = song;
      return true;
    }
  }
  return false;
}

void LibraryWatcher::DirectoryChanged(const QString &path) {
  qDebug() << path;
  if (!paths_needing_rescan_.contains(path))
    paths_needing_rescan_ << path;

  rescan_timer_->start();
}

void LibraryWatcher::RescanPathsNow() {
  foreach (const QString& path, paths_needing_rescan_)
    ScanDirectory(path);
  paths_needing_rescan_.clear();
}
