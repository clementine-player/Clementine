
#include "songtracker.h"

#include "core/filesystemwatcherinterface.h"
#include "core/logging.h"
#include "musicbrainz/chromaprinter.h"
#include "playlist/playlistmanager.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFutureWatcher>
#include <QMutableListIterator>
#include <QtConcurrentMap>
#include <QUrl>

static const int kChromaPlayLengthSecs = 10;
static const int kMinStreamLengthMSecs = 4000;
static const int kFallbackChunkSize = 512;
static const char* kFileFilter =
    "*.mp3 *.ogg *.flac *.mpc *.m4a *.m4b *.aac *.wma "
    "*.mp4 *.spx *.wav *.ape *.wv *.mka *.opus *.oga *.mp2";

SongTracker::SongTracker(PlaylistManager* manager)
    : QObject(nullptr),
      manager_(manager),
      fs_watcher_(FileSystemWatcherInterface::Create(this)) {
  connect(fs_watcher_, SIGNAL(PathChanged(const QString&)), this,
          SLOT(DirectoryChanged(const QString&)));

  QSettings settings;
  settings.beginGroup(Playlist::kSettingsGroup);
  enabled_ = settings.value("track_songs", false).toBool();
}

void SongTracker::SetEnabled(bool enable) {
  if (enabled_ == enable) return;

  enabled_ = enable;

  QSettings settings;
  settings.beginGroup(Playlist::kSettingsGroup);
  settings.setValue("track_songs", enable);
  settings.endGroup();

  if (enable) {
    for (Playlist* playlist : manager_->GetAllPlaylists()) {
      TrackAsync(playlist->id(), playlist->GetAllSongs());
    }
  } else {
    dir_map_.clear();
    fs_watcher_->Clear();
  }
}

void SongTracker::TrackAsync(int playlist_id, const SongList& songs) {
  metaObject()->invokeMethod(this, "Track", Qt::QueuedConnection,
                             Q_ARG(int, playlist_id), Q_ARG(SongList, songs));
}

void SongTracker::Track(int playlist_id, const SongList& songs) {
  if (!enabled_) return;

  for (const Song& song : songs) {
    const QUrl& url = song.url();
    if (url.scheme() != "file") return;

    const QString& fingerprint = GetFingerprint(url.toLocalFile());
    if (fingerprint.isEmpty()) {
      // Untrackable
      return;
    }

    // Watch directory of song
    const QFileInfo& fileInfo = QFileInfo(url.toLocalFile());
    const QString& dir_path = fileInfo.absoluteDir().path();
    if (!dir_map_.contains(dir_path)) {
      qLog(Debug) << "watching dir: " << dir_path;
      fs_watcher_->AddPath(dir_path);
    }
    const qint64 mtime = fileInfo.lastModified().toMSecsSinceEpoch();
    dir_map_.insert(dir_path, {url, fingerprint, mtime, playlist_id});
  }
}

void SongTracker::Untrack(int playlist_id, const Song& song) {
  const QUrl& url = song.url();
  const QString& dir_path = QFileInfo(url.toLocalFile()).absoluteDir().path();
  QHash<QString, WatchedSong>::iterator it = dir_map_.find(dir_path);
  while (it != dir_map_.end() && it.key() == dir_path) {
    if (it.value().url == url && it.value().playlist_id == playlist_id) {
      it = dir_map_.erase(it);
    } else {
      ++it;
    }
  }
  if (!dir_map_.contains(dir_path)) {
    qLog(Debug) << "unwatching dir: " << dir_path;
    fs_watcher_->RemovePath(dir_path);
  }
}

QString SongTracker::GetFingerprint(const QString& filename) {
  QMutexLocker l(&mutex_);

  const QFileInfo& fileInfo = QFileInfo(filename);
  if (!fileInfo.exists()) return QString();

  const qint64 last_modified = fileInfo.lastModified().toMSecsSinceEpoch();

  // Check if already created
  if (fn_map_.contains(filename) && last_modified == fn_map_[filename].mtime) {
    return fn_map_[filename].fingerprint;
  }

  QCryptographicHash hash(QCryptographicHash::Md5);

  Chromaprinter chromaprinter(filename);
  const QString& chroma_print =
      chromaprinter.CreateFingerprint(kChromaPlayLengthSecs, false);
  const qint32 length_msec = chromaprinter.GetLength();
  if (!chroma_print.isEmpty() && length_msec >= kMinStreamLengthMSecs) {
    hash.addData(chroma_print.toAscii());
  } else {
    // Fallback
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
      char databuf[kFallbackChunkSize];
      int readlen = 0;
      if ((readlen = file.read(databuf, kFallbackChunkSize)) > 0) {
        hash.addData(databuf, readlen);
      }
      file.close();
    }
  }

  // Add length
  if (length_msec > 0) {
    hash.addData(QString::number(length_msec).toAscii());
  }

  const QString& fingerprint(hash.result().toBase64());

  fn_map_[filename] = {fingerprint, last_modified};

  return fingerprint;
}

void SongTracker::DirectoryChanged(const QString& path) {
  const QList<WatchedSong>& watched_songs = dir_map_.values(path);
  dir_map_.remove(path);

  QHash<int, FoundSongs> playlist_map;
  for (WatchedSong watched_song : watched_songs) {
    const QFileInfo& fileInfo = QFileInfo(watched_song.url.toLocalFile());
    const qint64 mtime =
        fileInfo.exists() ? fileInfo.lastModified().toMSecsSinceEpoch() : -1;
    if (mtime == watched_song.mtime) {
      // Song did not change
      dir_map_.insert(path, watched_song);
      continue;
    }

    const QUrl& new_file_url =
        SearchDir(watched_song.fingerprint, watched_song.url);
    if (new_file_url.isEmpty()) {
      // Song is lost
      continue;
    }
    playlist_map[watched_song.playlist_id].insert(watched_song.url,
                                                  new_file_url);

    // Re-insert
    const QFileInfo& new_fileInfo = QFileInfo(new_file_url.toLocalFile());
    watched_song.url = new_file_url;
    watched_song.mtime = new_fileInfo.lastModified().toMSecsSinceEpoch();
    dir_map_.insert(new_fileInfo.absoluteDir().path(), watched_song);
  }

  if (!playlist_map.isEmpty())
    qLog(Debug) << "found files: " << playlist_map;

  QHashIterator<int, FoundSongs> it(playlist_map);
  while (it.hasNext()) {
    it.next();
    Playlist* playlist = manager_->playlist(it.key());
    if (playlist) {
      metaObject()->invokeMethod(playlist, "UpdateFilenames",
                                 Qt::QueuedConnection,
                                 Q_ARG(FoundSongs, it.value()));
    }
  }
}

QUrl SongTracker::SearchDir(const QString& fingerprint, const QUrl& old_url) {
  if (fingerprint.isEmpty()) return QUrl();

  QDir dir = QFileInfo(old_url.toLocalFile()).dir();
  if (dir.isReadable()) {
    dir.setNameFilters(QString(kFileFilter).split(" "));
    dir.setSorting(QDir::Time);
    dir.setFilter(QDir::Files | QDir::Readable);
    for (const QFileInfo& fileInfo : dir.entryInfoList()) {
      const QString& filename = fileInfo.filePath();
      const QString& fp = GetFingerprint(filename);
      if (fp == fingerprint) {
        return QUrl::fromLocalFile(filename);
      }
    }
  }

  return QUrl();
}
