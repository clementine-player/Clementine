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

#ifndef PLAYLISTBACKEND_H
#define PLAYLISTBACKEND_H

#include <QFuture>
#include <QList>
#include <QObject>

#include "playlistitem.h"

#include <boost/shared_ptr.hpp>

class Database;

class PlaylistBackend : public QObject {
  Q_OBJECT

 public:
  Q_INVOKABLE PlaylistBackend(QObject* parent = 0);
  void SetDatabase(boost::shared_ptr<Database> db) { db_ = db; }

  struct Playlist {
    int id;
    QString name;
    int last_played;
  };
  typedef QList<Playlist> PlaylistList;
  typedef QFuture<boost::shared_ptr<PlaylistItem> > PlaylistItemFuture;

  PlaylistList GetAllPlaylists();
  Playlist GetPlaylist(int id);
  PlaylistItemFuture GetPlaylistItems(int playlist);
  void SavePlaylistAsync(int playlist, const PlaylistItemList& items,
                         int last_played);
  void SetPlaylistOrder(const QList<int>& ids);

  int CreatePlaylist(const QString& name);
  void RemovePlaylist(int id);
  void RenamePlaylist(int id, const QString& new_name);

 public slots:
  void SavePlaylist(int playlist, const PlaylistItemList& items, int last_played);

 private:
  static boost::shared_ptr<PlaylistItem> NewSongFromQuery(const SqlRow& row);

  boost::shared_ptr<Database> db_;
};

#endif // PLAYLISTBACKEND_H
