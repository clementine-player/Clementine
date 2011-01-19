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

#include "playlistbackend.h"
#include "core/database.h"
#include "core/scopedtransaction.h"
#include "core/song.h"
#include "library/librarybackend.h"
#include "library/sqlrow.h"
#include "playlist/songplaylistitem.h"
#include "playlistparsers/cueparser.h"
#include "smartplaylists/generator.h"

#include <QFile>
#include <QHash>
#include <QMutexLocker>
#include <QSqlQuery>
#include <QtConcurrentMap>
#include <QtDebug>

#include <boost/bind.hpp>

using smart_playlists::GeneratorPtr;

using boost::shared_ptr;

PlaylistBackend::PlaylistBackend(QObject* parent)
  : QObject(parent),
    library_(NULL)
{
}

void PlaylistBackend::SetLibrary(LibraryBackend* library) {
  library_ = library;
}

PlaylistBackend::PlaylistList PlaylistBackend::GetAllPlaylists() {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  PlaylistList ret;

  QSqlQuery q("SELECT ROWID, name, last_played, dynamic_playlist_type,"
              "       dynamic_playlist_data, dynamic_playlist_backend"
              " FROM playlists", db);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return ret;

  while (q.next()) {
    Playlist p;
    p.id = q.value(0).toInt();
    p.name = q.value(1).toString();
    p.last_played = q.value(2).toInt();
    p.dynamic_type = q.value(3).toString();
    p.dynamic_data = q.value(4).toByteArray();
    p.dynamic_backend = q.value(5).toString();
    ret << p;
  }

  return ret;
}

PlaylistBackend::Playlist PlaylistBackend::GetPlaylist(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, name, last_played, dynamic_playlist_type,"
              "       dynamic_playlist_data, dynamic_playlist_backend"
              " FROM playlists"
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
  p.dynamic_type = q.value(3).toString();
  p.dynamic_data = q.value(4).toByteArray();
  p.dynamic_backend = q.value(5).toString();

  return p;
}

QFuture<PlaylistItemPtr> PlaylistBackend::GetPlaylistItems(int playlist) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QString query = "SELECT songs.ROWID, " + Song::JoinSpec("songs") + ","
                  "       magnatune_songs.ROWID, " + Song::JoinSpec("magnatune_songs") + ","
                  "       jamendo_songs.ROWID, " + Song::JoinSpec("jamendo_songs") + ","
                  "       p.type, p.url, p.title, p.artist, p.album, p.length,"
                  "       p.radio_service, p.beginning, p.cue_path"
                  " FROM playlist_items AS p"
                  " LEFT JOIN songs"
                  "    ON p.library_id = songs.ROWID"
                  " LEFT JOIN magnatune_songs"
                  "    ON p.library_id = magnatune_songs.ROWID"
                  " LEFT JOIN jamendo.songs AS jamendo_songs"
                  "    ON p.library_id = jamendo_songs.ROWID"
                  " WHERE p.playlist = :playlist";
  QSqlQuery q(query, db);

  q.bindValue(":playlist", playlist);
  q.exec();
  if (db_->CheckErrors(q.lastError()))
    return QFuture<PlaylistItemPtr>();

  QList<SqlRow> rows;

  while (q.next()) {
    rows << SqlRow(q);
  }

  // it's probable that we'll have a few songs associated with the
  // same CUE so we're caching results of parsing CUEs
  boost::shared_ptr<NewSongFromQueryState> state_ptr(new NewSongFromQueryState());
  return QtConcurrent::mapped(rows, boost::bind(&PlaylistBackend::NewSongFromQuery, this, _1, state_ptr));
}

PlaylistItemPtr PlaylistBackend::NewSongFromQuery(const SqlRow& row, boost::shared_ptr<NewSongFromQueryState> state) {
  // The song tables get joined first, plus one each for the song ROWIDs
  const int playlist_row = (Song::kColumns.count() + 1) * 3;

  PlaylistItemPtr item(PlaylistItem::NewFromType(row.value(playlist_row).toString()));
  if (item) {
    item->InitFromQuery(row);
    return RestoreCueData(item, state);
  } else {
    return item;
  }
}

// If song had a CUE and the CUE still exists, the metadata from it will
// be applied here.
PlaylistItemPtr PlaylistBackend::RestoreCueData(PlaylistItemPtr item, boost::shared_ptr<NewSongFromQueryState> state) {
  // we need library to run a CueParser; also, this method applies only to
  // file-type PlaylistItems
  if(!library_ || item->type() != "File") {
    return item;
  }
  CueParser cue_parser(library_);

  Song song = item->Metadata();
  // we're only interested in .cue songs here
  if(!song.has_cue()) {
    return item;
  }

  QString cue_path = song.cue_path();
  // if .cue was deleted - reload the song
  if(!QFile::exists(cue_path)) {
    item->Reload();
    return item;
  }

  SongList song_list;
  {
    QMutexLocker locker(&state->mutex_);

    if(!state->cached_cues_.contains(cue_path)) {
      QFile cue(cue_path);
      cue.open(QIODevice::ReadOnly);

      song_list = cue_parser.Load(&cue, cue_path, QDir(cue_path.section('/', 0, -2)));
      state->cached_cues_[cue_path] = song_list;
    } else {
      song_list = state->cached_cues_[cue_path];
    }
  }

  foreach(const Song& from_list, song_list) {
    if(from_list.filename() == song.filename() &&
       from_list.beginning() == song.beginning()) {
      // we found a matching section; replace the input
      // item with a new one containing CUE metadata
      return PlaylistItemPtr(new SongPlaylistItem(from_list));
    }
  }

  // there's no such section in the related .cue -> reload the song
  item->Reload();
  return item;
}

void PlaylistBackend::SavePlaylistAsync(int playlist, const PlaylistItemList &items,
                                        int last_played, GeneratorPtr dynamic) {
  metaObject()->invokeMethod(this, "SavePlaylist", Qt::QueuedConnection,
                             Q_ARG(int, playlist),
                             Q_ARG(PlaylistItemList, items),
                             Q_ARG(int, last_played),
                             Q_ARG(smart_playlists::GeneratorPtr, dynamic));
}

void PlaylistBackend::SavePlaylist(int playlist, const PlaylistItemList& items,
                                   int last_played, GeneratorPtr dynamic) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery clear("DELETE FROM playlist_items WHERE playlist = :playlist", db);
  QSqlQuery insert("INSERT INTO playlist_items"
                   " (playlist, type, library_id, url, title, artist, album,"
                   "  length, radio_service, beginning, cue_path)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", db);
  QSqlQuery update("UPDATE playlists SET "
                   "   last_played=:last_played,"
                   "   dynamic_playlist_type=:dynamic_type,"
                   "   dynamic_playlist_data=:dynamic_data,"
                   "   dynamic_playlist_backend=:dynamic_backend"
                   " WHERE ROWID=:playlist", db);

  ScopedTransaction transaction(&db);

  // Clear the existing items in the playlist
  clear.bindValue(":playlist", playlist);
  clear.exec();
  if (db_->CheckErrors(clear.lastError()))
    return;

  // Save the new ones
  foreach (PlaylistItemPtr item, items) {
    insert.bindValue(0, playlist);
    item->BindToQuery(&insert);

    insert.exec();
    db_->CheckErrors(insert.lastError());
  }

  // Update the last played track number
  update.bindValue(":last_played", last_played);
  if (dynamic) {
    update.bindValue(":dynamic_type", dynamic->type());
    update.bindValue(":dynamic_data", dynamic->Save());
    update.bindValue(":dynamic_backend", dynamic->library()->songs_table());
  } else {
    update.bindValue(":dynamic_type", QString());
    update.bindValue(":dynamic_data", QByteArray());
    update.bindValue(":dynamic_backend", QString());
  }
  update.bindValue(":playlist", playlist);
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
