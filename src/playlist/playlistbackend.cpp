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

#include <memory>
#include <functional>

#include <QFile>
#include <QHash>
#include <QMutexLocker>
#include <QSqlQuery>
#include <QtDebug>

#include "core/application.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/scopedtransaction.h"
#include "core/song.h"
#include "library/librarybackend.h"
#include "library/sqlrow.h"
#include "playlist/songplaylistitem.h"
#include "playlistparsers/cueparser.h"
#include "smartplaylists/generator.h"

using std::placeholders::_1;
using std::shared_ptr;

using smart_playlists::GeneratorPtr;

const int PlaylistBackend::kSongTableJoins = 4;

PlaylistBackend::PlaylistBackend(Application* app, QObject* parent)
    : QObject(parent), app_(app), db_(app_->database()) {}

PlaylistBackend::PlaylistList PlaylistBackend::GetAllPlaylists() {
  return GetPlaylists(GetPlaylists_All);
}

PlaylistBackend::PlaylistList PlaylistBackend::GetAllOpenPlaylists() {
  return GetPlaylists(GetPlaylists_OpenInUi);
}

PlaylistBackend::PlaylistList PlaylistBackend::GetAllFavoritePlaylists() {
  return GetPlaylists(GetPlaylists_Favorite);
}

PlaylistBackend::PlaylistList PlaylistBackend::GetPlaylists(
    GetPlaylistsFlags flags) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  PlaylistList ret;

  QStringList condition_list;
  if (flags & GetPlaylists_OpenInUi) {
    condition_list << "ui_order != -1";
  }
  if (flags & GetPlaylists_Favorite) {
    condition_list << "is_favorite != 0";
  }
  QString condition;
  if (!condition_list.isEmpty()) {
    condition = " WHERE " + condition_list.join(" OR ");
  }

  QSqlQuery q(
      "SELECT ROWID, name, last_played, dynamic_playlist_type,"
      "       dynamic_playlist_data, dynamic_playlist_backend,"
      "       special_type, ui_path, is_favorite"
      " FROM playlists"
      " " +
          condition + " ORDER BY ui_order",
      db);
  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    Playlist p;
    p.id = q.value(0).toInt();
    p.name = q.value(1).toString();
    p.last_played = q.value(2).toInt();
    p.dynamic_type = q.value(3).toString();
    p.dynamic_data = q.value(4).toByteArray();
    p.dynamic_backend = q.value(5).toString();
    p.special_type = q.value(6).toString();
    p.ui_path = q.value(7).toString();
    p.favorite = q.value(8).toBool();
    ret << p;
  }

  return ret;
}

PlaylistBackend::Playlist PlaylistBackend::GetPlaylist(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(
      "SELECT ROWID, name, last_played, dynamic_playlist_type,"
      "       dynamic_playlist_data, dynamic_playlist_backend,"
      "       special_type, ui_path, is_favorite"
      " FROM playlists"
      " WHERE ROWID=:id",
      db);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q)) return Playlist();

  q.next();

  Playlist p;
  p.id = q.value(0).toInt();
  p.name = q.value(1).toString();
  p.last_played = q.value(2).toInt();
  p.dynamic_type = q.value(3).toString();
  p.dynamic_data = q.value(4).toByteArray();
  p.dynamic_backend = q.value(5).toString();
  p.special_type = q.value(6).toString();
  p.ui_path = q.value(7).toString();
  p.favorite = q.value(8).toBool();

  return p;
}

QSqlQuery PlaylistBackend::GetPlaylistRows(int playlist) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QString query = "SELECT songs.ROWID, " + Song::JoinSpec("songs") +
                  ","
                  "       magnatune_songs.ROWID, " +
                  Song::JoinSpec("magnatune_songs") +
                  ","
                  "       jamendo_songs.ROWID, " +
                  Song::JoinSpec("jamendo_songs") +
                  ","
                  "       p.ROWID, " +
                  Song::JoinSpec("p") +
                  ","
                  "       p.type, p.radio_service"
                  " FROM playlist_items AS p"
                  " LEFT JOIN songs"
                  "    ON p.library_id = songs.ROWID"
                  " LEFT JOIN magnatune_songs"
                  "    ON p.library_id = magnatune_songs.ROWID"
                  " LEFT JOIN jamendo.songs AS jamendo_songs"
                  "    ON p.library_id = jamendo_songs.ROWID"
                  " WHERE p.playlist = :playlist";
  QSqlQuery q(db);
  // Forward iterations only may be faster
  q.setForwardOnly(true);
  q.prepare(query);
  q.bindValue(":playlist", playlist);
  q.exec();

  return q;
}

QList<PlaylistItemPtr> PlaylistBackend::GetPlaylistItems(int playlist) {
  QSqlQuery q = GetPlaylistRows(playlist);
  // Note that as this only accesses the query, not the db, we don't need the
  // mutex.
  if (db_->CheckErrors(q)) return QList<PlaylistItemPtr>();

  // it's probable that we'll have a few songs associated with the
  // same CUE so we're caching results of parsing CUEs
  std::shared_ptr<NewSongFromQueryState> state_ptr(new NewSongFromQueryState());
  QList<PlaylistItemPtr> playlistitems;
  while (q.next()) {
    playlistitems << NewPlaylistItemFromQuery(SqlRow(q), state_ptr);
  }
  return playlistitems;
}

QList<Song> PlaylistBackend::GetPlaylistSongs(int playlist) {
  QSqlQuery q = GetPlaylistRows(playlist);
  // Note that as this only accesses the query, not the db, we don't need the
  // mutex.
  if (db_->CheckErrors(q)) return QList<Song>();

  // it's probable that we'll have a few songs associated with the
  // same CUE so we're caching results of parsing CUEs
  std::shared_ptr<NewSongFromQueryState> state_ptr(new NewSongFromQueryState());
  QList<Song> songs;
  while (q.next()) {
    songs << NewSongFromQuery(SqlRow(q), state_ptr);
  }
  return songs;
}

PlaylistItemPtr PlaylistBackend::NewPlaylistItemFromQuery(
    const SqlRow& row, std::shared_ptr<NewSongFromQueryState> state) {
  // The song tables get joined first, plus one each for the song ROWIDs
  const int playlist_row = (Song::kColumns.count() + 1) * kSongTableJoins;

  PlaylistItemPtr item(
      PlaylistItem::NewFromType(row.value(playlist_row).toString()));
  if (item) {
    item->InitFromQuery(row);
    return RestoreCueData(item, state);
  } else {
    return item;
  }
}

Song PlaylistBackend::NewSongFromQuery(
    const SqlRow& row, std::shared_ptr<NewSongFromQueryState> state) {
  return NewPlaylistItemFromQuery(row, state)->Metadata();
}

// If song had a CUE and the CUE still exists, the metadata from it will
// be applied here.
PlaylistItemPtr PlaylistBackend::RestoreCueData(
    PlaylistItemPtr item, std::shared_ptr<NewSongFromQueryState> state) {
  // we need library to run a CueParser; also, this method applies only to
  // file-type PlaylistItems
  if (item->type() != "File") {
    return item;
  }
  CueParser cue_parser(app_->library_backend());

  Song song = item->Metadata();
  // we're only interested in .cue songs here
  if (!song.has_cue()) {
    return item;
  }

  QString cue_path = song.cue_path();
  // if .cue was deleted - reload the song
  if (!QFile::exists(cue_path)) {
    item->Reload();
    return item;
  }

  SongList song_list;
  {
    QMutexLocker locker(&state->mutex_);

    if (!state->cached_cues_.contains(cue_path)) {
      QFile cue(cue_path);
      cue.open(QIODevice::ReadOnly);

      song_list =
          cue_parser.Load(&cue, cue_path, QDir(cue_path.section('/', 0, -2)));
      state->cached_cues_[cue_path] = song_list;
    } else {
      song_list = state->cached_cues_[cue_path];
    }
  }

  for (const Song& from_list : song_list) {
    if (from_list.url().toEncoded() == song.url().toEncoded() &&
        from_list.beginning_nanosec() == song.beginning_nanosec()) {
      // we found a matching section; replace the input
      // item with a new one containing CUE metadata
      return PlaylistItemPtr(new SongPlaylistItem(from_list));
    }
  }

  // there's no such section in the related .cue -> reload the song
  item->Reload();
  return item;
}

void PlaylistBackend::SavePlaylistAsync(int playlist,
                                        const PlaylistItemList& items,
                                        int last_played, GeneratorPtr dynamic) {
  metaObject()->invokeMethod(
      this, "SavePlaylist", Qt::QueuedConnection, Q_ARG(int, playlist),
      Q_ARG(PlaylistItemList, items), Q_ARG(int, last_played),
      Q_ARG(smart_playlists::GeneratorPtr, dynamic));
}

void PlaylistBackend::SavePlaylist(int playlist, const PlaylistItemList& items,
                                   int last_played, GeneratorPtr dynamic) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  qLog(Debug) << "Saving playlist" << playlist;

  QSqlQuery clear("DELETE FROM playlist_items WHERE playlist = :playlist", db);
  QSqlQuery insert(
      "INSERT INTO playlist_items"
      " (playlist, type, library_id, radio_service, " +
          Song::kColumnSpec +
          ")"
          " VALUES (:playlist, :type, :library_id, :radio_service, " +
          Song::kBindSpec + ")",
      db);
  QSqlQuery update(
      "UPDATE playlists SET "
      "   last_played=:last_played,"
      "   dynamic_playlist_type=:dynamic_type,"
      "   dynamic_playlist_data=:dynamic_data,"
      "   dynamic_playlist_backend=:dynamic_backend"
      " WHERE ROWID=:playlist",
      db);

  ScopedTransaction transaction(&db);

  // Clear the existing items in the playlist
  clear.bindValue(":playlist", playlist);
  clear.exec();
  if (db_->CheckErrors(clear)) return;

  // Save the new ones
  for (PlaylistItemPtr item : items) {
    insert.bindValue(":playlist", playlist);
    item->BindToQuery(&insert);

    insert.exec();
    db_->CheckErrors(insert);
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
  if (db_->CheckErrors(update)) return;

  transaction.Commit();
}

int PlaylistBackend::CreatePlaylist(const QString& name,
                                    const QString& special_type) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(
      "INSERT INTO playlists (name, special_type)"
      " VALUES (:name, :special_type)",
      db);
  q.bindValue(":name", name);
  q.bindValue(":special_type", special_type);
  q.exec();
  if (db_->CheckErrors(q)) return -1;

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
  if (db_->CheckErrors(delete_playlist)) return;

  delete_items.exec();
  if (db_->CheckErrors(delete_items)) return;

  transaction.Commit();
}

void PlaylistBackend::RenamePlaylist(int id, const QString& new_name) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  QSqlQuery q("UPDATE playlists SET name=:name WHERE ROWID=:id", db);
  q.bindValue(":name", new_name);
  q.bindValue(":id", id);

  q.exec();
  db_->CheckErrors(q);
}

void PlaylistBackend::FavoritePlaylist(int id, bool is_favorite) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  QSqlQuery q("UPDATE playlists SET is_favorite=:is_favorite WHERE ROWID=:id",
              db);
  q.bindValue(":is_favorite", is_favorite ? 1 : 0);
  q.bindValue(":id", id);

  q.exec();
  db_->CheckErrors(q);
}

void PlaylistBackend::SetPlaylistOrder(const QList<int>& ids) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  ScopedTransaction transaction(&db);

  QSqlQuery q("UPDATE playlists SET ui_order=-1", db);
  q.exec();
  if (db_->CheckErrors(q)) return;

  q = QSqlQuery("UPDATE playlists SET ui_order=:index WHERE ROWID=:id", db);
  for (int i = 0; i < ids.count(); ++i) {
    q.bindValue(":index", i);
    q.bindValue(":id", ids[i]);
    q.exec();
    if (db_->CheckErrors(q)) return;
  }

  transaction.Commit();
}

void PlaylistBackend::SetPlaylistUiPath(int id, const QString& path) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  QSqlQuery q("UPDATE playlists SET ui_path=:path WHERE ROWID=:id", db);

  ScopedTransaction transaction(&db);

  q.bindValue(":path", path);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q)) return;

  transaction.Commit();
}
