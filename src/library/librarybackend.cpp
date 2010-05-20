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

#include "librarybackend.h"
#include "libraryquery.h"
#include "core/database.h"
#include "core/scopedtransaction.h"

#include <QDir>
#include <QVariant>
#include <QSettings>
#include <QtDebug>
#include <QCoreApplication>

LibraryBackend::LibraryBackend(Database *db, const QString& songs_table,
                               const QString& dirs_table,
                               const QString& subdirs_table, QObject *parent)
  : QObject(parent),
    db_(db),
    songs_table_(songs_table),
    dirs_table_(dirs_table),
    subdirs_table_(subdirs_table)
{
}

void LibraryBackend::LoadDirectoriesAsync() {
  metaObject()->invokeMethod(this, "LoadDirectories", Qt::QueuedConnection);
}

void LibraryBackend::UpdateTotalSongCountAsync() {
  metaObject()->invokeMethod(this, "UpdateTotalSongCount", Qt::QueuedConnection);
}

void LibraryBackend::LoadDirectories() {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("SELECT ROWID, path FROM %1").arg(dirs_table_), db);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return;

  while (q.next()) {
    Directory dir;
    dir.id = q.value(0).toInt();
    dir.path = q.value(1).toString();

    emit DirectoryDiscovered(dir, SubdirsInDirectory(dir.id, db));
  }
}

SubdirectoryList LibraryBackend::SubdirsInDirectory(int id) {
  QSqlDatabase db = db_->Connect();
  return SubdirsInDirectory(id, db);
}

SubdirectoryList LibraryBackend::SubdirsInDirectory(int id, QSqlDatabase &db) {
  QSqlQuery q(QString("SELECT path, mtime FROM %1"
                      " WHERE directory = :dir").arg(subdirs_table_), db);
  q.bindValue(":dir", id);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return SubdirectoryList();

  SubdirectoryList subdirs;
  while (q.next()) {
    Subdirectory subdir;
    subdir.directory_id = id;
    subdir.path = q.value(0).toString();
    subdir.mtime = q.value(1).toUInt();
    subdirs << subdir;
  }

  return subdirs;
}

void LibraryBackend::UpdateTotalSongCount() {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("SELECT COUNT(*) FROM %1").arg(songs_table_), db);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return;
  if (!q.next()) return;

  emit TotalSongCountUpdated(q.value(0).toInt());
}

void LibraryBackend::AddDirectory(const QString &path) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("INSERT INTO %1 (path, subdirs)"
                      " VALUES (:path, 1)").arg(dirs_table_), db);
  q.bindValue(":path", path);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return;

  Directory dir;
  dir.path = path;
  dir.id = q.lastInsertId().toInt();

  emit DirectoryDiscovered(dir, SubdirectoryList());
}

void LibraryBackend::RemoveDirectory(const Directory& dir) {
  QSqlDatabase db(db_->Connect());

  // Remove songs first
  DeleteSongs(FindSongsInDirectory(dir.id));

  ScopedTransaction transaction(&db);

  // Delete the subdirs that were in this directory
  QSqlQuery q(QString("DELETE FROM %1 WHERE directory = :id")
              .arg(subdirs_table_), db);
  q.bindValue(":id", dir.id);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return;

  // Now remove the directory itself
  q = QSqlQuery(QString("DELETE FROM %1 WHERE ROWID = :id")
                .arg(dirs_table_), db);
  q.bindValue(":id", dir.id);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return;

  emit DirectoryDeleted(dir);

  transaction.Commit();
}

SongList LibraryBackend::FindSongsInDirectory(int id) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("SELECT ROWID, " + Song::kColumnSpec +
                      " FROM %1 WHERE directory = :directory")
              .arg(songs_table_), db);
  q.bindValue(":directory", id);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return SongList();

  SongList ret;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q);
    ret << song;
  }
  return ret;
}

void LibraryBackend::AddOrUpdateSubdirs(const SubdirectoryList& subdirs) {
  QSqlDatabase db(db_->Connect());
  QSqlQuery find_query(QString("SELECT ROWID FROM %1"
                               " WHERE directory = :id AND path = :path")
                       .arg(subdirs_table_), db);
  QSqlQuery add_query(QString("INSERT INTO %1 (directory, path, mtime)"
                              " VALUES (:id, :path, :mtime)")
                      .arg(subdirs_table_), db);
  QSqlQuery update_query(QString("UPDATE %1 SET mtime = :mtime"
                                 " WHERE directory = :id AND path = :path")
                         .arg(subdirs_table_), db);
  QSqlQuery delete_query(QString("DELETE FROM %1"
                                 " WHERE directory = :id AND path = :path")
                         .arg(subdirs_table_), db);

  ScopedTransaction transaction(&db);
  foreach (const Subdirectory& subdir, subdirs) {
    if (subdir.mtime == 0) {
      // Delete the subdirectory
      delete_query.bindValue(":id", subdir.directory_id);
      delete_query.bindValue(":path", subdir.path);
      delete_query.exec();
      db_->CheckErrors(delete_query.lastError());
    } else {
      // See if this subdirectory already exists in the database
      find_query.bindValue(":id", subdir.directory_id);
      find_query.bindValue(":path", subdir.path);
      find_query.exec();
      if (db_->CheckErrors(find_query.lastError())) continue;

      if (find_query.next()) {
        update_query.bindValue(":mtime", subdir.mtime);
        update_query.bindValue(":id", subdir.directory_id);
        update_query.bindValue(":path", subdir.path);
        update_query.exec();
        db_->CheckErrors(update_query.lastError());
      } else {
        add_query.bindValue(":id", subdir.directory_id);
        add_query.bindValue(":path", subdir.path);
        add_query.bindValue(":mtime", subdir.mtime);
        add_query.exec();
        db_->CheckErrors(add_query.lastError());
      }
    }
  }
  transaction.Commit();
}

void LibraryBackend::AddOrUpdateSongs(const SongList& songs) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery check_dir(QString("SELECT ROWID FROM %1 WHERE ROWID = :id")
                      .arg(dirs_table_), db);
  QSqlQuery add_song(QString("INSERT INTO %1 (" + Song::kColumnSpec + ")"
                             " VALUES (" + Song::kBindSpec + ")")
                     .arg(songs_table_), db);
  QSqlQuery update_song(QString("UPDATE %1 SET " + Song::kUpdateSpec +
                                " WHERE ROWID = :id").arg(songs_table_), db);

  ScopedTransaction transaction(&db);

  SongList added_songs;
  SongList deleted_songs;

  foreach (const Song& song, songs) {
    // Do a sanity check first - make sure the song's directory still exists
    // This is to fix a possible race condition when a directory is removed
    // while LibraryWatcher is scanning it.
    if (!dirs_table_.isEmpty()) {
      check_dir.bindValue(":id", song.directory_id());
      check_dir.exec();
      if (db_->CheckErrors(check_dir.lastError())) continue;

      if (!check_dir.next())
        continue; // Directory didn't exist
    }


    if (song.id() == -1) {
      // Create
      song.BindToQuery(&add_song);
      add_song.exec();
      if (db_->CheckErrors(add_song.lastError())) continue;

      Song copy(song);
      copy.set_id(add_song.lastInsertId().toInt());
      added_songs << copy;
    } else {
      // Get the previous song data first
      Song old_song(GetSongById(song.id()));
      if (!old_song.is_valid())
        continue;

      // Update
      song.BindToQuery(&update_song);
      update_song.bindValue(":id", song.id());
      update_song.exec();
      if (db_->CheckErrors(update_song.lastError())) continue;

      deleted_songs << old_song;
      added_songs << song;
    }
  }

  transaction.Commit();

  if (!deleted_songs.isEmpty())
    emit SongsDeleted(deleted_songs);

  if (!added_songs.isEmpty())
    emit SongsDiscovered(added_songs);

  UpdateTotalSongCountAsync();
}

void LibraryBackend::UpdateMTimesOnly(const SongList& songs) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("UPDATE %1 SET mtime = :mtime WHERE ROWID = :id")
              .arg(songs_table_), db);

  ScopedTransaction transaction(&db);
  foreach (const Song& song, songs) {
    q.bindValue(":mtime", song.mtime());
    q.bindValue(":id", song.id());
    q.exec();
    db_->CheckErrors(q.lastError());
  }
  transaction.Commit();
}

void LibraryBackend::DeleteSongs(const SongList &songs) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("DELETE FROM %1 WHERE ROWID = :id")
              .arg(songs_table_), db);

  ScopedTransaction transaction(&db);
  foreach (const Song& song, songs) {
    q.bindValue(":id", song.id());
    q.exec();
    db_->CheckErrors(q.lastError());
  }
  transaction.Commit();

  emit SongsDeleted(songs);

  UpdateTotalSongCountAsync();
}

QStringList LibraryBackend::GetAllArtists(const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT artist");
  query.AddCompilationRequirement(false);

  if (!ExecQuery(&query)) return QStringList();

  QStringList ret;
  while (query.Next()) {
    ret << query.Value(0).toString();
  }
  return ret;
}

LibraryBackend::AlbumList LibraryBackend::GetAllAlbums(const QueryOptions &opt) {
  return GetAlbums(QString(), false, opt);
}

LibraryBackend::AlbumList LibraryBackend::GetAlbumsByArtist(const QString& artist,
                                                            const QueryOptions& opt) {
  return GetAlbums(artist, false, opt);
}

SongList LibraryBackend::GetSongs(const QString& artist, const QString& album, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("ROWID, " + Song::kColumnSpec);
  query.AddCompilationRequirement(false);
  query.AddWhere("artist", artist);
  query.AddWhere("album", album);

  if (!ExecQuery(&query)) return SongList();

  SongList ret;
  while (query.Next()) {
    Song song;
    song.InitFromQuery(query);
    ret << song;
  }
  return ret;
}

Song LibraryBackend::GetSongById(int id) {
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("SELECT ROWID, " + Song::kColumnSpec + " FROM %1"
                      " WHERE ROWID = :id").arg(songs_table_), db);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return Song();

  q.next();

  Song ret;
  ret.InitFromQuery(q);
  return ret;
}

bool LibraryBackend::HasCompilations(const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("ROWID");
  query.AddCompilationRequirement(true);

  if (!ExecQuery(&query)) return false;

  return query.Next();
}

LibraryBackend::AlbumList LibraryBackend::GetCompilationAlbums(const QueryOptions& opt) {
  return GetAlbums(QString(), true, opt);
}

SongList LibraryBackend::GetCompilationSongs(const QString& album, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("ROWID, " + Song::kColumnSpec);
  query.AddCompilationRequirement(true);
  query.AddWhere("album", album);

  if (!ExecQuery(&query)) return SongList();

  SongList ret;
  while (query.Next()) {
    Song song;
    song.InitFromQuery(query);
    ret << song;
  }
  return ret;
}

void LibraryBackend::UpdateCompilations() {
  QSqlDatabase db(db_->Connect());

  // Look for albums that have songs by more than one artist in the same
  // directory

  QSqlQuery q("SELECT artist, album, filename, sampler FROM songs ORDER BY album", db);
  q.exec();
  if (db_->CheckErrors(q.lastError())) return;

  QMap<QString, CompilationInfo> compilation_info;
  while (q.next()) {
    QString artist = q.value(0).toString();
    QString album = q.value(1).toString();
    QString filename = q.value(2).toString();
    bool sampler = q.value(3).toBool();

    // Ignore songs that don't have an album field set
    if (album.isEmpty())
      continue;

    // Find the directory the song is in
    QDir dir(filename);
    QString path = QDir::toNativeSeparators(dir.canonicalPath());
    int last_separator = path.lastIndexOf(QDir::separator());

    if (last_separator == -1)
      continue;

    path = path.left(last_separator);

    CompilationInfo& info = compilation_info[album];
    info.artists.insert(artist);
    info.directories.insert(path);
    if (sampler) info.has_samplers = true;
    else         info.has_not_samplers = true;
  }

  // Now mark the songs that we think are in compilations
  QSqlQuery update(QString("UPDATE %1"
                           " SET sampler = :sampler,"
                           "     effective_compilation = ((compilation OR :sampler OR forced_compilation_on) AND NOT forced_compilation_off) + 0"
                           " WHERE album = :album").arg(songs_table_), db);
  QSqlQuery find_songs(QString("SELECT ROWID, " + Song::kColumnSpec + " FROM %1"
                               " WHERE album = :album AND sampler = :sampler")
                       .arg(songs_table_), db);

  SongList deleted_songs;
  SongList added_songs;

  ScopedTransaction transaction(&db);

  QMap<QString, CompilationInfo>::const_iterator it = compilation_info.constBegin();
  for ( ; it != compilation_info.constEnd() ; ++it) {
    const CompilationInfo& info = it.value();
    QString album(it.key());

    // If there were more artists than there were directories for this album,
    // then it's a compilation

    if (info.artists.count() > info.directories.count()) {
      if (info.has_not_samplers)
        UpdateCompilations(find_songs, update, deleted_songs, added_songs, album, 1);
    } else {
      if (info.has_samplers)
        UpdateCompilations(find_songs, update, deleted_songs, added_songs, album, 0);
    }
  }

  transaction.Commit();

  if (!deleted_songs.isEmpty()) {
    emit SongsDeleted(deleted_songs);
    emit SongsDiscovered(added_songs);
  }
}

void LibraryBackend::UpdateCompilations(QSqlQuery& find_songs, QSqlQuery& update,
                                        SongList& deleted_songs, SongList& added_songs,
                                        const QString& album, int sampler) {
  // Get songs that were already in that album, so we can tell the model
  // they've been updated
  find_songs.bindValue(":album", album);
  find_songs.bindValue(":sampler", int(!sampler));
  find_songs.exec();
  while (find_songs.next()) {
    Song song;
    song.InitFromQuery(find_songs);
    deleted_songs << song;
    song.set_sampler(true);
    added_songs << song;
  }

  // Mark this album
  update.bindValue(":sampler", sampler);
  update.bindValue(":album", album);
  update.exec();
  db_->CheckErrors(update.lastError());
}

LibraryBackend::AlbumList LibraryBackend::GetAlbums(const QString& artist,
                                                    bool compilation,
                                                    const QueryOptions& opt) {
  AlbumList ret;

  LibraryQuery query(opt);
  query.SetColumnSpec("album, artist, compilation, sampler, art_automatic, art_manual");
  query.SetOrderBy("album");

  if (compilation) {
    query.AddCompilationRequirement(true);
  } else if (!artist.isNull()) {
    query.AddCompilationRequirement(false);
    query.AddWhere("artist", artist);
  }

  if (!ExecQuery(&query)) return ret;

  QString last_album;
  QString last_artist;
  while (query.Next()) {
    bool compilation = query.Value(2).toBool() | query.Value(3).toBool();

    Album info;
    info.artist = compilation ? QString() : query.Value(1).toString();
    info.album_name = query.Value(0).toString();
    info.art_automatic = query.Value(4).toString();
    info.art_manual = query.Value(5).toString();

    if (info.artist == last_artist && info.album_name == last_album)
      continue;

    ret << info;

    last_album = info.album_name;
    last_artist = info.artist;
  }

  return ret;
}

LibraryBackend::Album LibraryBackend::GetAlbumArt(const QString& artist, const QString& album) {
  Album ret;
  ret.album_name = album;
  ret.artist = artist;

  LibraryQuery query = LibraryQuery(QueryOptions());
  query.SetColumnSpec("art_automatic, art_manual");
  query.AddWhere("artist", artist);
  query.AddWhere("album", album);

   if (!ExecQuery(&query)) return ret;

  if (query.Next()) {
    ret.art_automatic = query.Value(0).toString();
    ret.art_manual = query.Value(1).toString();
  }

  return ret;
}

void LibraryBackend::UpdateManualAlbumArtAsync(const QString &artist,
                                               const QString &album,
                                               const QString &art) {
  metaObject()->invokeMethod(this, "UpdateManualAlbumArt", Qt::QueuedConnection,
                             Q_ARG(QString, artist),
                             Q_ARG(QString, album),
                             Q_ARG(QString, art));
}

void LibraryBackend::UpdateManualAlbumArt(const QString &artist,
                                          const QString &album,
                                          const QString &art) {
  QSqlDatabase db(db_->Connect());

  QString sql(QString("UPDATE %1 SET art_manual = :art"
                      " WHERE album = :album").arg(songs_table_));
  if (!artist.isNull())
    sql += " AND artist = :artist";

  QSqlQuery q(sql, db);
  q.bindValue(":art", art);
  q.bindValue(":album", album);
  if (!artist.isNull())
    q.bindValue(":artist", artist);

  q.exec();
  db_->CheckErrors(q.lastError());
}

void LibraryBackend::ForceCompilation(const QString& artist, const QString& album, bool on) {
  QSqlDatabase db(db_->Connect());

  // Get the songs before they're updated
  LibraryQuery query;
  query.SetColumnSpec("ROWID, " + Song::kColumnSpec);
  query.AddWhere("album", album);
  if (!artist.isNull())
    query.AddWhere("artist", artist);

  if (!ExecQuery(&query)) return;

  SongList deleted_songs;
  while (query.Next()) {
    Song song;
    song.InitFromQuery(query);
    deleted_songs << song;
  }

  // Update the songs
  QString sql(QString("UPDATE %1 SET forced_compilation_on = :forced_compilation_on,"
                      "              forced_compilation_off = :forced_compilation_off,"
                      "              effective_compilation = ((compilation OR sampler OR :forced_compilation_on) AND NOT :forced_compilation_off) + 0"
                      " WHERE album = :album").arg(songs_table_));
  if (!artist.isEmpty())
    sql += " AND artist = :artist";

  QSqlQuery q(sql, db);
  q.bindValue(":forced_compilation_on", on ? 1 : 0);
  q.bindValue(":forced_compilation_off", on ? 0 : 1);
  q.bindValue(":album", album);
  if (!artist.isEmpty())
    q.bindValue(":artist", artist);

  q.exec();
  db_->CheckErrors(q.lastError());

  // Now get the updated songs
  if (!ExecQuery(&query)) return;

  SongList added_songs;
  while (query.Next()) {
    Song song;
    song.InitFromQuery(query);
    added_songs << song;
  }

  if (!added_songs.isEmpty() || !deleted_songs.isEmpty()) {
    emit SongsDeleted(deleted_songs);
    emit SongsDiscovered(added_songs);
  }
}

bool LibraryBackend::ExecQuery(LibraryQuery *q) {
  return !db_->CheckErrors(q->Exec(db_->Connect(), songs_table_));
}
