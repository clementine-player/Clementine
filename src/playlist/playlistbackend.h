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

#ifndef PLAYLISTBACKEND_H
#define PLAYLISTBACKEND_H

#include <QFuture>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QObject>

#include "playlistitem.h"
#include "smartplaylists/generator_fwd.h"

class Application;
class Database;

class PlaylistBackend : public QObject {
  Q_OBJECT

 public:
  Q_INVOKABLE PlaylistBackend(Application* app, QObject* parent = 0);

  struct Playlist {
    Playlist()
      : id(-1),
        favorite(false),
        last_played(0) {
    }

    int id;
    QString name;
    QString ui_path;
    bool favorite;
    int last_played;
    QString dynamic_type;
    QString dynamic_backend;
    QByteArray dynamic_data;

    // Special playlists have different behaviour, eg. the "spotify-search"
    // type has a spotify search box at the top, replacing the ordinary filter.
    QString special_type;
  };
  typedef QList<Playlist> PlaylistList;
  typedef QFuture<PlaylistItemPtr> PlaylistItemFuture;

  static const int kSongTableJoins;

  PlaylistList GetAllPlaylists();
  PlaylistList GetAllOpenPlaylists();
  PlaylistList GetAllFavoritePlaylists();
  PlaylistBackend::Playlist GetPlaylist(int id);
  PlaylistItemFuture GetPlaylistItems(int playlist);
  QFuture<Song> GetPlaylistSongs(int playlist);

  void SetPlaylistOrder(const QList<int>& ids);
  void SetPlaylistUiPath(int id, const QString& path);

  int CreatePlaylist(const QString& name, const QString& special_type);
  void SavePlaylistAsync(int playlist, const PlaylistItemList& items,
                         int last_played, smart_playlists::GeneratorPtr dynamic);
  void RenamePlaylist(int id, const QString& new_name);
  void FavoritePlaylist(int id, bool is_favorite);
  void RemovePlaylist(int id);

  Application* app() const { return app_; }

 public slots:
  void SavePlaylist(int playlist, const PlaylistItemList& items,
                    int last_played, smart_playlists::GeneratorPtr dynamic);

 private:
  struct NewSongFromQueryState {
    QHash<QString, SongList> cached_cues_;
    QMutex mutex_;
  };

  QList<SqlRow> GetPlaylistRows(int playlist);

  Song NewSongFromQuery(const SqlRow& row, std::shared_ptr<NewSongFromQueryState> state);
  PlaylistItemPtr NewPlaylistItemFromQuery(const SqlRow& row, std::shared_ptr<NewSongFromQueryState> state);
  PlaylistItemPtr RestoreCueData(PlaylistItemPtr item, std::shared_ptr<NewSongFromQueryState> state);

  enum GetPlaylistsFlags {
    GetPlaylists_OpenInUi = 1,
    GetPlaylists_Favorite = 2,
    GetPlaylists_All = GetPlaylists_OpenInUi | GetPlaylists_Favorite
  };
  PlaylistList GetPlaylists(GetPlaylistsFlags flags);

  Application* app_;
  Database* db_;
};

#endif // PLAYLISTBACKEND_H
