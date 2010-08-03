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
#include "library/sqlrow.h"

#include <QSqlQuery>
#include <QtConcurrentMap>
#include <QtDebug>

using boost::shared_ptr;

PlaylistBackend::PlaylistBackend(QObject* parent)
  : QObject(parent)
{
}

PlaylistBackend::PlaylistList PlaylistBackend::GetAllPlaylists() {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  PlaylistList ret;

  QSqlQuery q("SELECT ROWID, name, last_played FROM playlists ORDER BY ui_order", db);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return ret;

  while (q.next()) {
    Playlist p;
    p.id = q.value(0).toInt();
    p.name = q.value(1).toString();
    p.last_played = q.value(2).toInt();
    ret << p;
  }

  return ret;
}

PlaylistBackend::Playlist PlaylistBackend::GetPlaylist(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, name, last_played FROM playlists"
              " WHERE ROWID=:id", db);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return Playlist();

  q.next();

  Playlist p;
  p.id = q.value(0).toInt();
  p.name = q.value(1).toString();
  p.last_played = q.value(2).toInt();

  return p;
}

QFuture<shared_ptr<PlaylistItem> > PlaylistBackend::GetPlaylistItems(int playlist) {
  QMutexLocker l(db_->Mutex());
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
    return QFuture<shared_ptr<PlaylistItem> >();

  QList<SqlRow> rows;

  while (q.next()) {
    rows << SqlRow(q);
  }

  return QtConcurrent::mapped(rows, &PlaylistBackend::NewSongFromQuery);
}

shared_ptr<PlaylistItem> PlaylistBackend::NewSongFromQuery(const SqlRow& row) {
  // The song tables gets joined first, plus one each for the song ROWIDs
  const int playlist_row = (Song::kColumns.count() + 1) * 2;

  shared_ptr<PlaylistItem> item(
      PlaylistItem::NewFromType(row.value(playlist_row).toString()));
  if (item) {
    item->InitFromQuery(row);
  }
  return item;
}

void PlaylistBackend::SavePlaylistAsync(int playlist, const PlaylistItemList &items,
                                        int last_played) {
  metaObject()->invokeMethod(this, "SavePlaylist", Qt::QueuedConnection,
                             Q_ARG(int, playlist),
                             Q_ARG(PlaylistItemList, items),
                             Q_ARG(int, last_played));
}

void PlaylistBackend::SavePlaylist(int playlist, const PlaylistItemList& items,
                                   int last_played) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery clear("DELETE FROM playlist_items WHERE playlist = :playlist", db);
  QSqlQuery insert("INSERT INTO playlist_items"
                   " (playlist, type, library_id, url, title, artist, album,"
                   "  length, radio_service)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", db);
  QSqlQuery update("UPDATE playlists SET last_played=:last_played"
                   " WHERE ROWID=:playlist", db);

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

  // Update the last played track number
  update.bindValue(":last_played", last_played);
  update.bindValue(":id", playlist);
  update.exec();
  if (db_->CheckErrors(update.lastError()))
    return;

  transaction.Commit();
}

int PlaylistBackend::CreatePlaylist(const QString &name) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("INSERT INTO playlists (name) VALUES (:name)", db);
  q.bindValue(":name", name);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return -1;

  return q.lastInsertId().toInt();
}

void PlaylistBackend::RemovePlaylist(int id) {
  QMutexLocker l(db_->Mutex());
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
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  QSqlQuery q("UPDATE playlists SET name=:name WHERE ROWID=:id", db);
  q.bindValue(":name", new_name);
  q.bindValue(":id", id);

  q.exec();
  db_->CheckErrors(q.lastError());
}

void PlaylistBackend::SetPlaylistOrder(const QList<int>& ids) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  QSqlQuery q("UPDATE playlists SET ui_order=:index WHERE ROWID=:id", db);

  ScopedTransaction transaction(&db);

  for (int i=0 ; i<ids.count() ; ++i) {
    q.bindValue(":index", i);
    q.bindValue(":id", ids[i]);
    q.exec();
    if (db_->CheckErrors(q.lastError()))
      return;
  }

  transaction.Commit();
}
