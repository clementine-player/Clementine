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
  QSqlDatabase db(db_->Connect());

  PlaylistList ret;

  QSqlQuery q("SELECT ROWID, name FROM playlists", db);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return ret;

  while (q.next()) {
    Playlist p;
    p.id = q.value(0).toInt();
    p.name = q.value(1).toString();
    ret << p;
  }

  return ret;
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

int PlaylistBackend::CreatePlaylist(const QString &name) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("INSERT INTO playlists (name) VALUES (:name)", db);
  q.bindValue(":name", name);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return -1;

  return q.lastInsertId().toInt();
}

void PlaylistBackend::RemovePlaylist(int id) {
  QSqlDatabase db(db_->Connect());
  QSqlQuery delete_playlist("DELETE FROM playlists WHERE ROWID=:id", db);
  QSqlQuery delete_items("DELETE FROM playlist_items WHERE playlist=:id", db);

  delete_playlist.bindValue(":id", id);
  delete_items.bindValue(":id", id);

  ScopedTransaction transaction(&db);

  delete_playlist.exec();
  if (db_->CheckErrors(delete_playlist.lastError()))
    return;

  delete_items.exec();
  if (db_->CheckErrors(delete_items.lastError()))
    return;

  transaction.Commit();
}

void PlaylistBackend::RenamePlaylist(int id, const QString &new_name) {
  QSqlDatabase db(db_->Connect());
  QSqlQuery q("UPDATE playlists SET name=:name WHERE ROWID=:id", db);
  q.bindValue(":name", new_name);
  q.bindValue(":id", id);

  q.exec();
  db_->CheckErrors(q.lastError());
}
