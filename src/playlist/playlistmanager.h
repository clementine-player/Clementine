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

#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QMap>

#include "core/song.h"

class LibraryBackend;
class Playlist;
class PlaylistBackend;
class PlaylistParser;
class PlaylistSequence;

class QModelIndex;
class QUrl;

class PlaylistManager : public QObject {
  Q_OBJECT

public:
  PlaylistManager(QObject *parent = 0);
  ~PlaylistManager();

  int current_id() const { return current_; }
  int active_id() const { return active_; }

  Playlist* playlist(int id) const { return playlists_[id].p; }
  Playlist* current() const { return playlist(current_id()); }
  Playlist* active() const { return playlist(active_id()); }

  QString name(int index) const { return playlists_[index].name; }

  void Init(LibraryBackend* library_backend, PlaylistBackend* playlist_backend,
            PlaylistSequence* sequence);

  LibraryBackend* library_backend() const { return library_backend_; }
  PlaylistBackend* playlist_backend() const { return playlist_backend_; }
  PlaylistSequence* sequence() const { return sequence_; }
  PlaylistParser* parser() const { return parser_; }

public slots:
  void New(const QString& name, const SongList& songs = SongList());
  void Load(const QString& filename);
  void Save(int id, const QString& filename);
  void Rename(int id, const QString& new_name);
  void Remove(int id);
  void ChangePlaylistOrder(const QList<int>& ids);

  void SetCurrentPlaylist(int id);
  void SetActivePlaylist(int id);
  void SetActiveToCurrent() { SetActivePlaylist(current_id()); }

  // Convenience slots that defer to either current() or active()
  void ClearCurrent();
  void ShuffleCurrent();
  void SetActivePlaying();
  void SetActivePaused();
  void SetActiveStopped();
  void SetActiveStreamMetadata(const QUrl& url, const Song& song);

signals:
  void PlaylistAdded(int id, const QString& name);
  void PlaylistRemoved(int id);
  void PlaylistRenamed(int id, const QString& new_name);
  void CurrentChanged(Playlist* new_playlist);
  void ActiveChanged(Playlist* new_playlist);

  void Error(const QString& message);

  // Forwarded from individual playlists
  void CurrentSongChanged(const Song& song);
  void PlaylistChanged();
  void EditingFinished(const QModelIndex& index);

private:
  Playlist* AddPlaylist(int id, const QString& name);

private:
  struct Data {
    Data(Playlist* _p = NULL, const QString& _name = QString()) : p(_p), name(_name) {}
    Playlist* p;
    QString name;
  };

  PlaylistBackend* playlist_backend_;
  LibraryBackend* library_backend_;
  PlaylistSequence* sequence_;
  PlaylistParser* parser_;

  // key = id
  QMap<int, Data> playlists_;

  int current_;
  int active_;
};

#endif // PLAYLISTMANAGER_H
