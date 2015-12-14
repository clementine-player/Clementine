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

#include <QColor>
#include <QItemSelection>
#include <QMap>
#include <QObject>
#include <QSettings>

#include "core/song.h"
#include "playlist.h"
#include "smartplaylists/generator_fwd.h"

class Application;
class LibraryBackend;
class PlaylistBackend;
class PlaylistContainer;
class PlaylistParser;
class PlaylistSequence;
class TaskManager;

class QModelIndex;
class QUrl;

class PlaylistManagerInterface : public QObject {
  Q_OBJECT

 public:
  PlaylistManagerInterface(Application* app, QObject* parent)
      : QObject(parent) {}

  virtual int current_id() const = 0;
  virtual int active_id() const = 0;

  virtual Playlist* playlist(int id) const = 0;
  virtual Playlist* current() const = 0;
  virtual Playlist* active() const = 0;

  // Returns the collection of playlists managed by this PlaylistManager.
  virtual QList<Playlist*> GetAllPlaylists() const = 0;
  // Grays out and reloads all deleted songs in all playlists.
  virtual void InvalidateDeletedSongs() = 0;
  // Removes all deleted songs from all playlists.
  virtual void RemoveDeletedSongs() = 0;

  virtual QItemSelection selection(int id) const = 0;
  virtual QItemSelection current_selection() const = 0;
  virtual QItemSelection active_selection() const = 0;

  virtual QString GetPlaylistName(int index) const = 0;

  virtual LibraryBackend* library_backend() const = 0;
  virtual PlaylistBackend* playlist_backend() const = 0;
  virtual PlaylistSequence* sequence() const = 0;
  virtual PlaylistParser* parser() const = 0;
  virtual PlaylistContainer* playlist_container() const = 0;

 public slots:
  virtual void New(const QString& name, const SongList& songs = SongList(),
                   const QString& special_type = QString()) = 0;
  virtual void Load(const QString& filename) = 0;
  virtual void Save(int id, const QString& filename,
                    Playlist::Path path_type) = 0;
  virtual void Rename(int id, const QString& new_name) = 0;
  virtual void Delete(int id) = 0;
  virtual bool Close(int id) = 0;
  virtual void Open(int id) = 0;
  virtual void ChangePlaylistOrder(const QList<int>& ids) = 0;

  virtual void SongChangeRequestProcessed(const QUrl& url, bool valid) = 0;

  virtual void SetCurrentPlaylist(int id) = 0;
  virtual void SetActivePlaylist(int id) = 0;
  virtual void SetActiveToCurrent() = 0;

  virtual void SelectionChanged(const QItemSelection& selection) = 0;

  // Convenience slots that defer to either current() or active()
  virtual void ClearCurrent() = 0;
  virtual void ShuffleCurrent() = 0;
  virtual void RemoveDuplicatesCurrent() = 0;
  virtual void RemoveUnavailableCurrent() = 0;
  virtual void SetActivePlaying() = 0;
  virtual void SetActivePaused() = 0;
  virtual void SetActiveStopped() = 0;
  virtual void SetActiveStreamMetadata(const QUrl& url, const Song& song) = 0;
  // Rate current song using 0.0 - 1.0 scale.
  virtual void RateCurrentSong(double rating) = 0;
  // Rate current song using 0 - 5 scale.
  virtual void RateCurrentSong(int rating) = 0;

  virtual void PlaySmartPlaylist(smart_playlists::GeneratorPtr generator,
                                 bool as_new, bool clear) = 0;

signals:
  void PlaylistManagerInitialized();

  void PlaylistAdded(int id, const QString& name, bool favorite);
  void PlaylistDeleted(int id);
  void PlaylistClosed(int id);
  void PlaylistRenamed(int id, const QString& new_name);
  void PlaylistFavorited(int id, bool favorite);
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
};

class PlaylistManager : public PlaylistManagerInterface {
  Q_OBJECT

 public:
  PlaylistManager(Application* app, QObject* parent = nullptr);
  ~PlaylistManager();

  int current_id() const { return current_; }
  int active_id() const { return active_; }

  Playlist* playlist(int id) const { return playlists_[id].p; }
  Playlist* current() const { return playlist(current_id()); }
  Playlist* active() const { return playlist(active_id()); }

  // Returns the collection of playlists managed by this PlaylistManager.
  QList<Playlist*> GetAllPlaylists() const;
  // Grays out and reloads all deleted songs in all playlists.
  void InvalidateDeletedSongs();
  // Removes all deleted songs from all playlists.
  void RemoveDeletedSongs();
  // Returns true if the playlist is open
  bool IsPlaylistOpen(int id);

  // Returns a pretty automatic name for playlist created from the given list of
  // songs.
  static QString GetNameForNewPlaylist(const SongList& songs);

  QItemSelection selection(int id) const;
  QItemSelection current_selection() const { return selection(current_id()); }
  QItemSelection active_selection() const { return selection(active_id()); }

  QString GetPlaylistName(int index) const { return playlists_[index].name; }
  bool IsPlaylistFavorite(int index) const {
    return playlists_[index].p->is_favorite();
  }

  void Init(LibraryBackend* library_backend, PlaylistBackend* playlist_backend,
            PlaylistSequence* sequence, PlaylistContainer* playlist_container);

  LibraryBackend* library_backend() const { return library_backend_; }
  PlaylistBackend* playlist_backend() const { return playlist_backend_; }
  PlaylistSequence* sequence() const { return sequence_; }
  PlaylistParser* parser() const { return parser_; }
  PlaylistContainer* playlist_container() const { return playlist_container_; }

 public slots:
  void New(const QString& name, const SongList& songs = SongList(),
           const QString& special_type = QString());
  void Load(const QString& filename);
  void Save(int id, const QString& filename, Playlist::Path path_type);
  // Display a file dialog to let user choose a file before saving the file
  void SaveWithUI(int id, const QString& suggested_filename);
  void Rename(int id, const QString& new_name);
  void Favorite(int id, bool favorite);
  void Delete(int id);
  bool Close(int id);
  void Open(int id);
  void ChangePlaylistOrder(const QList<int>& ids);

  void SetCurrentPlaylist(int id);
  void SetActivePlaylist(int id);
  void SetActiveToCurrent();

  void SelectionChanged(const QItemSelection& selection);

  // Makes a playlist current if it's open already, or opens it and makes it
  // current if it is hidden.
  void SetCurrentOrOpen(int id);

  // Convenience slots that defer to either current() or active()
  void ClearCurrent();
  void ShuffleCurrent();
  void RemoveDuplicatesCurrent();
  void RemoveUnavailableCurrent();
  void SetActiveStreamMetadata(const QUrl& url, const Song& song);
  // Rate current song using 0.0 - 1.0 scale.
  void RateCurrentSong(double rating);
  // Rate current song using 0 - 5 scale.
  void RateCurrentSong(int rating);

  void PlaySmartPlaylist(smart_playlists::GeneratorPtr generator, bool as_new,
                         bool clear);

  void SongChangeRequestProcessed(const QUrl& url, bool valid);

  void InsertUrls(int id, const QList<QUrl>& urls, int pos = -1,
                  bool play_now = false, bool enqueue = false);
  void InsertSongs(int id, const SongList& songs, int pos = -1,
                   bool play_now = false, bool enqueue = false);
  // Removes items with given indices from the playlist. This operation is not
  // undoable.
  void RemoveItemsWithoutUndo(int id, const QList<int>& indices);

 private slots:
  void SetActivePlaying();
  void SetActivePaused();
  void SetActiveStopped();

  void OneOfPlaylistsChanged();
  void UpdateSummaryText();
  void SongsDiscovered(const SongList& songs);
  void ItemsLoadedForSavePlaylist(QFuture<SongList> future,
                                  const QString& filename,
                                  Playlist::Path path_type);

 private:
  Playlist* AddPlaylist(int id, const QString& name,
                        const QString& special_type, const QString& ui_path,
                        bool favorite);

 private:
  struct Data {
    Data(Playlist* _p = nullptr, const QString& _name = QString())
        : p(_p), name(_name) {}
    Playlist* p;
    QString name;
    QItemSelection selection;
  };

  Application* app_;
  PlaylistBackend* playlist_backend_;
  LibraryBackend* library_backend_;
  PlaylistSequence* sequence_;
  PlaylistParser* parser_;
  PlaylistContainer* playlist_container_;

  // key = id
  QMap<int, Data> playlists_;

  int current_;
  int active_;
};

#endif  // PLAYLISTMANAGER_H
