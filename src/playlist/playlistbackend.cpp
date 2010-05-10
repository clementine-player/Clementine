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

#include "playlistbackend.h"
#include "core/database.h"
#include "core/scopedtransaction.h"
#include "core/song.h"

#include <QtDebug>
#include <QSqlQuery>

using boost::shared_ptr;

PlaylistBackend::PlaylistBackend(Database* db, QObject* parent)
  : QObject(parent),
    db_(db)
{
}

PlaylistBackend::PlaylistList PlaylistBackend::GetAllPlaylists() {
  qWarning() << "Not implemented:" << __PRETTY_FUNCTION__;
  return PlaylistList();
}

PlaylistItemList PlaylistBackend::GetPlaylistItems(int playlist) {
  QSqlDatabase db(db_->Connect());

  PlaylistItemList ret;

  QSqlQuery q("SELECT songs.ROWID, " + Song::JoinSpec("songs") + ","
              "       magnatune_songs.ROWID, " + Song::JoinSpec("magnatune_songs") + ","
              "       p.type, p.url, p.title, p.artist, p.album, p.length,"
              "       p.radio_service"
              " FROM playlist_items AS p"
              " LEFT JOIN songs"
              "    ON p.library_id = songs.ROWID"
              " LEFT JOIN magnatune_songs"
              "    ON p.library_id = magnatune_songs.ROWID"
              " WHERE p.playlist = :playlist", db);
  q.bindValue(":playlist", playlist);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return ret;

  while (q.next()) {
    // The song tables gets joined first, plus one each for the song ROWIDs
    const int row = (Song::kColumns.count() + 1) * 2;

    shared_ptr<PlaylistItem> item(
        PlaylistItem::NewFromType(q.value(row + 0).toString()));
    if (!item)
      continue;

    if (item->InitFromQuery(q))
      ret << item;
  }

  return ret;
}

void PlaylistBackend::SavePlaylistAsync(int playlist, const PlaylistItemList &items) {
  metaObject()->invokeMethod(this, "SavePlaylist", Qt::QueuedConnection,
                             Q_ARG(int, playlist),
                             Q_ARG(PlaylistItemList, items));
}

void PlaylistBackend::SavePlaylist(int playlist, const PlaylistItemList& items) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery clear("DELETE FROM playlist_items WHERE playlist = :playlist", db);
  QSqlQuery insert("INSERT INTO playlist_items"
                   " (playlist, type, library_id, url, title, artist, album,"
                   "  length, radio_service)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", db);

  ScopedTransaction transaction(&db);

  // Clear the existing items in the playlist
  clear.bindValue(":playlist", playlist);
  clear.exec();
  if (db_->CheckErrors(clear.lastError()))
    return;

  // Save the new ones
  foreach (shared_ptr<PlaylistItem> item, items) {
    insert.bindValue(0, playlist);
    item->BindToQuery(&insert);

    insert.exec();
    db_->CheckErrors(insert.lastError());
  }

  transaction.Commit();
}
