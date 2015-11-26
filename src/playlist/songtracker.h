/* This file is part of Clementine.
 * Copyright 2015, David Sansome <me@davidsansome.com>
 *
 * Clementine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Clementine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SONGTRACKER_H
#define SONGTRACKER_H

#include "core/song.h"

#include <QMultiHash>
#include <QMutex>
#include <QObject>
#include <QUrl>

class FileSystemWatcherInterface;
class PlaylistManager;

// Track local files automatically by fingerprinting them and watching for
// directory changes.
// Internal maps are used to avoid unnecessary duplicate operations, so it
// is recommended to use one instance of this class for the whole application.
class SongTracker : public QObject {
  Q_OBJECT

 public:
  SongTracker(PlaylistManager* manager);

  void SetEnabled(bool enabled);
  bool IsEnabled() { return enabled_; }
  // Track file movements for songs in a playlist.
  // Songs might not have been fully loaded.
  void TrackAsync(int playlist_id, const SongList& songs);
  // void Untrack(int playlist_id, const SongList& songs);
  void Untrack(int playlist_id, const Song& song);
  // void UntrackAll();
  // Get the fingerprint of a song file. Empty string is returned if file does
  // not exist.
  QString GetFingerprint(const QString& filename);

 private slots:
  void DirectoryChanged(const QString& path);
  void Track(int playlist_id, const SongList& songs);

 private:
  struct WatchedSong {
    QUrl url;
    QString fingerprint;
    qint64 mtime;
    int playlist_id;
  };
  struct CreatedFingerprint {
    QString fingerprint;
    qint64 mtime;
  };

  QUrl SearchDir(const QString& fingerprint, const QUrl& old_url);

  PlaylistManager* manager_;
  FileSystemWatcherInterface* fs_watcher_;

  // directory path to watched songs in dir
  QMultiHash<QString, WatchedSong> dir_map_;
  // filename to fingerprint cache
  QHash<QString, CreatedFingerprint> fn_map_;

  QMutex mutex_;
  bool enabled_;
};

// old filename to new filename
typedef QHash<QUrl, QUrl> FoundSongs;
Q_DECLARE_METATYPE(FoundSongs)

#endif  // SONGTRACKER_H
