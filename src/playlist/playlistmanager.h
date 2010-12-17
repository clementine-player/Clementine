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

#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QItemSelection>
#include <QMap>
#include <QObject>

#include "core/song.h"
#include "smartplaylists/generator_fwd.h"

class LibraryBackend;
class Playlist;
class PlaylistBackend;
class PlaylistParser;
class PlaylistSequence;
class TaskManager;

class QModelIndex;
class QUrl;

class PlaylistManager : public QObject {
  Q_OBJECT

public:
  PlaylistManager(TaskManager* task_manager, QObject *parent = 0);
  ~PlaylistManager();

  int current_id() const { return current_; }
  int active_id() const { return active_; }

  Playlist* playlist(int id) const { return playlists_[id].p; }
  Playlist* current() const { return playlist(current_id()); }
  Playlist* active() const { return playlist(active_id()); }

  const QItemSelection& selection(int id) const { return playlists_[id].selection; }
  const QItemSelection& current_selection() const { return selection(current_id()); }
  const QItemSelection& active_selection() const { return selection(active_id()); }

  QString name(int index) const { return playlists_[index].name; }

  void Init(LibraryBackend* library_backend, PlaylistBackend* playlist_backend,
            PlaylistSequence* sequence);

  TaskManager* task_manager() const { return task_manager_; }
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

  void SelectionChanged(const QItemSelection& selection);

  // Convenience slots that defer to either current() or active()
  void ClearCurrent();
  void ShuffleCurrent();
  void SetActivePlaying();
  void SetActivePaused();
  void SetActiveStopped();
  void SetActiveStreamMetadata(const QUrl& url, const Song& song);
  // Rate current song using 0.0 - 1.0 scale.
  void RateCurrentSong(double rating);
  // Rate current song using 0 - 5 scale.
  void RateCurrentSong(int rating);

  void PlaySmartPlaylist(smart_playlists::GeneratorPtr generator, bool as_new, bool clear);

signals:
  void PlaylistManagerInitialized();

  void PlaylistAdded(int id, const QString& name);
  void PlaylistRemoved(int id);
  void PlaylistRenamed(int id, const QString& new_name);
  void CurrentChanged(Playlist* new_playlist);
  void ActiveChanged(Playlist* new_playlist);

  void Error(const QString& message);
  void SummaryTextChanged(const QString& summary);

  // Forwarded from individual playlists
  void CurrentSongChanged(const Song& song);
  
  // Signals that one of manager's playlists has changed (new items, new
  // ordering etc.) - the argument shows which.
  void PlaylistChanged(Playlist* playlist);
  void EditingFinished(const QModelIndex& index);
  void PlayRequested(const QModelIndex& index);

private slots:
  void OneOfPlaylistsChanged();
  void UpdateSummaryText();
  void SongsDiscovered(const SongList& songs);
  void LoadFinished(bool success);

private:
  Playlist* AddPlaylist(int id, const QString& name);

private:
  struct Data {
    Data(Playlist* _p = NULL, const QString& _name = QString()) : p(_p), name(_name) {}
    Playlist* p;
    QString name;
    QItemSelection selection;
  };

  TaskManager* task_manager_;
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
