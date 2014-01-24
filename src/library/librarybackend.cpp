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

#include "librarybackend.h"
#include "libraryquery.h"
#include "sqlrow.h"
#include "core/application.h"
#include "core/database.h"
#include "core/scopedtransaction.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"
#include "smartplaylists/search.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QVariant>
#include <QtDebug>

const char* LibraryBackend::kSettingsGroup = "LibraryBackend";

const char* LibraryBackend::kNewScoreSql =
    "case when playcount <= 0 then (%1 * 100 + score) / 2"
    "     else (score * (playcount + skipcount) + %1 * 100) / (playcount + skipcount + 1)"
    " end";

LibraryBackend::LibraryBackend(QObject *parent)
  : LibraryBackendInterface(parent),
    save_statistics_in_file_(false),
    save_ratings_in_file_(false)
{
}

void LibraryBackend::Init(Database* db, const QString& songs_table,
                          const QString& dirs_table, const QString& subdirs_table,
                          const QString& fts_table) {
  db_ = db;
  songs_table_ = songs_table;
  dirs_table_ = dirs_table;
  subdirs_table_ = subdirs_table;
  fts_table_ = fts_table;
  ReloadSettings();
}

void LibraryBackend::LoadDirectoriesAsync() {
  metaObject()->invokeMethod(this, "LoadDirectories", Qt::QueuedConnection);
}

void LibraryBackend::UpdateTotalSongCountAsync() {
  metaObject()->invokeMethod(this, "UpdateTotalSongCount", Qt::QueuedConnection);
}

void LibraryBackend::IncrementPlayCountAsync(int id) {
  metaObject()->invokeMethod(this, "IncrementPlayCount", Qt::QueuedConnection,
                             Q_ARG(int, id));
}

void LibraryBackend::IncrementSkipCountAsync(int id, float progress) {
  metaObject()->invokeMethod(this, "IncrementSkipCount", Qt::QueuedConnection,
                             Q_ARG(int, id), Q_ARG(float, progress));
}

void LibraryBackend::ResetStatisticsAsync(int id) {
  metaObject()->invokeMethod(this, "ResetStatistics", Qt::QueuedConnection,
                             Q_ARG(int, id));
}

void LibraryBackend::UpdateSongRatingAsync(int id, float rating) {
  metaObject()->invokeMethod(this, "UpdateSongRating", Qt::QueuedConnection,
                             Q_ARG(int, id), Q_ARG(float, rating));
}

void LibraryBackend::LoadDirectories() {
  DirectoryList dirs = GetAllDirectories();

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  foreach (const Directory& dir, dirs) {
    emit DirectoryDiscovered(dir, SubdirsInDirectory(dir.id, db));
  }
}

void LibraryBackend::ChangeDirPath(int id, const QString& old_path,
                                   const QString& new_path) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  ScopedTransaction t(&db);

  // Do the dirs table
  QSqlQuery q(QString("UPDATE %1 SET path=:path WHERE ROWID=:id").arg(dirs_table_), db);
  q.bindValue(":path", new_path);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q)) return;

  const QByteArray old_url = QUrl::fromLocalFile(old_path).toEncoded();
  const QByteArray new_url = QUrl::fromLocalFile(new_path).toEncoded();

  const int path_len = old_url.length();

  // Do the subdirs table
  q = QSqlQuery(QString("UPDATE %1 SET path=:path || substr(path, %2)"
                        " WHERE directory=:id").arg(subdirs_table_).arg(path_len), db);
  q.bindValue(":path", new_url);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q)) return;

  // Do the songs table
  q = QSqlQuery(QString("UPDATE %1 SET filename=:path || substr(filename, %2)"
                        " WHERE directory=:id").arg(songs_table_).arg(path_len), db);
  q.bindValue(":path", new_url);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q)) return;

  t.Commit();
}

DirectoryList LibraryBackend::GetAllDirectories() {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  DirectoryList ret;

  QSqlQuery q(QString("SELECT ROWID, path FROM %1").arg(dirs_table_), db);
  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    Directory dir;
    dir.id = q.value(0).toInt();
    dir.path = q.value(1).toString();

    ret << dir;
  }
  return ret;
}

SubdirectoryList LibraryBackend::SubdirsInDirectory(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db = db_->Connect();
  return SubdirsInDirectory(id, db);
}

SubdirectoryList LibraryBackend::SubdirsInDirectory(int id, QSqlDatabase &db) {
  QSqlQuery q(QString("SELECT path, mtime FROM %1"
                      " WHERE directory = :dir").arg(subdirs_table_), db);
  q.bindValue(":dir", id);
  q.exec();
  if (db_->CheckErrors(q)) return SubdirectoryList();

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
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("SELECT COUNT(*) FROM %1 WHERE unavailable = 0").arg(songs_table_), db);
  q.exec();
  if (db_->CheckErrors(q)) return;
  if (!q.next()) return;

  emit TotalSongCountUpdated(q.value(0).toInt());
}

void LibraryBackend::AddDirectory(const QString& path) {
  QString canonical_path = QFileInfo(path).canonicalFilePath();
  QString db_path = canonical_path;

  if (Application::kIsPortable
   && Utilities::UrlOnSameDriveAsClementine(QUrl::fromLocalFile(canonical_path))) {
    db_path = Utilities::GetRelativePathToClementineBin(QUrl::fromLocalFile(db_path)).toLocalFile();
    qLog(Debug) << "db_path" << db_path;
  }

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("INSERT INTO %1 (path, subdirs)"
                      " VALUES (:path, 1)").arg(dirs_table_), db);
  q.bindValue(":path", db_path);
  q.exec();
  if (db_->CheckErrors(q)) return;

  Directory dir;
  dir.path = canonical_path;
  dir.id = q.lastInsertId().toInt();

  emit DirectoryDiscovered(dir, SubdirectoryList());
}

void LibraryBackend::RemoveDirectory(const Directory& dir) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  // Remove songs first
  DeleteSongs(FindSongsInDirectory(dir.id));

  ScopedTransaction transaction(&db);

  // Delete the subdirs that were in this directory
  QSqlQuery q(QString("DELETE FROM %1 WHERE directory = :id")
              .arg(subdirs_table_), db);
  q.bindValue(":id", dir.id);
  q.exec();
  if (db_->CheckErrors(q)) return;

  // Now remove the directory itself
  q = QSqlQuery(QString("DELETE FROM %1 WHERE ROWID = :id")
                .arg(dirs_table_), db);
  q.bindValue(":id", dir.id);
  q.exec();
  if (db_->CheckErrors(q)) return;

  emit DirectoryDeleted(dir);

  transaction.Commit();
}

SongList LibraryBackend::FindSongsInDirectory(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("SELECT ROWID, " + Song::kColumnSpec +
                      " FROM %1 WHERE directory = :directory")
              .arg(songs_table_), db);
  q.bindValue(":directory", id);
  q.exec();
  if (db_->CheckErrors(q)) return SongList();

  SongList ret;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q, true);
    ret << song;
  }
  return ret;
}

void LibraryBackend::AddOrUpdateSubdirs(const SubdirectoryList& subdirs) {
  QMutexLocker l(db_->Mutex());
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
      db_->CheckErrors(delete_query);
    } else {
      // See if this subdirectory already exists in the database
      find_query.bindValue(":id", subdir.directory_id);
      find_query.bindValue(":path", subdir.path);
      find_query.exec();
      if (db_->CheckErrors(find_query)) continue;

      if (find_query.next()) {
        update_query.bindValue(":mtime", subdir.mtime);
        update_query.bindValue(":id", subdir.directory_id);
        update_query.bindValue(":path", subdir.path);
        update_query.exec();
        db_->CheckErrors(update_query);
      } else {
        add_query.bindValue(":id", subdir.directory_id);
        add_query.bindValue(":path", subdir.path);
        add_query.bindValue(":mtime", subdir.mtime);
        add_query.exec();
        db_->CheckErrors(add_query);
      }
    }
  }
  transaction.Commit();
}

void LibraryBackend::AddOrUpdateSongs(const SongList& songs) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery check_dir(QString("SELECT ROWID FROM %1 WHERE ROWID = :id")
                      .arg(dirs_table_), db);
  QSqlQuery add_song(QString("INSERT INTO %1 (" + Song::kColumnSpec + ")"
                             " VALUES (" + Song::kBindSpec + ")")
                     .arg(songs_table_), db);
  QSqlQuery update_song(QString("UPDATE %1 SET " + Song::kUpdateSpec +
                                " WHERE ROWID = :id").arg(songs_table_), db);
  QSqlQuery add_song_fts(QString("INSERT INTO %1 (ROWID, " + Song::kFtsColumnSpec + ")"
                                 " VALUES (:id, " + Song::kFtsBindSpec + ")")
                         .arg(fts_table_), db);
  QSqlQuery update_song_fts(QString("UPDATE %1 SET " + Song::kFtsUpdateSpec +
                                    " WHERE ROWID = :id").arg(fts_table_), db);

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
      if (db_->CheckErrors(check_dir)) continue;

      if (!check_dir.next())
        continue; // Directory didn't exist
    }

    if (song.id() == -1) {
      // Create

      // Insert the row and create a new ID
      song.BindToQuery(&add_song);
      add_song.exec();
      if (db_->CheckErrors(add_song)) continue;

      // Get the new ID
      const int id = add_song.lastInsertId().toInt();

      // Add to the FTS index
      add_song_fts.bindValue(":id", id);
      song.BindToFtsQuery(&add_song_fts);
      add_song_fts.exec();
      if (db_->CheckErrors(add_song_fts)) continue;

      Song copy(song);
      copy.set_id(id);
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
      if (db_->CheckErrors(update_song)) continue;

      song.BindToFtsQuery(&update_song_fts);
      update_song_fts.bindValue(":id", song.id());
      update_song_fts.exec();
      if (db_->CheckErrors(update_song_fts)) continue;

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
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("UPDATE %1 SET mtime = :mtime WHERE ROWID = :id")
              .arg(songs_table_), db);

  ScopedTransaction transaction(&db);
  foreach (const Song& song, songs) {
    q.bindValue(":mtime", song.mtime());
    q.bindValue(":id", song.id());
    q.exec();
    db_->CheckErrors(q);
  }
  transaction.Commit();
}

void LibraryBackend::DeleteSongs(const SongList &songs) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery remove(QString("DELETE FROM %1 WHERE ROWID = :id")
                   .arg(songs_table_), db);
  QSqlQuery remove_fts(QString("DELETE FROM %1 WHERE ROWID = :id")
                       .arg(fts_table_), db);

  ScopedTransaction transaction(&db);
  foreach (const Song& song, songs) {
    remove.bindValue(":id", song.id());
    remove.exec();
    db_->CheckErrors(remove);

    remove_fts.bindValue(":id", song.id());
    remove_fts.exec();
    db_->CheckErrors(remove_fts);
  }
  transaction.Commit();

  emit SongsDeleted(songs);

  UpdateTotalSongCountAsync();
}

void LibraryBackend::MarkSongsUnavailable(const SongList &songs) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery remove(QString("UPDATE %1 SET unavailable = 1 WHERE ROWID = :id")
                   .arg(songs_table_), db);

  ScopedTransaction transaction(&db);
  foreach (const Song& song, songs) {
    remove.bindValue(":id", song.id());
    remove.exec();
    db_->CheckErrors(remove);
  }
  transaction.Commit();

  emit SongsDeleted(songs);
  UpdateTotalSongCountAsync();
}

QStringList LibraryBackend::GetAll(const QString& column, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT " + column);
  query.AddCompilationRequirement(false);

  QMutexLocker l(db_->Mutex());
  if (!ExecQuery(&query)) return QStringList();

  QStringList ret;
  while (query.Next()) {
    ret << query.Value(0).toString();
  }
  return ret;
}

QStringList LibraryBackend::GetAllArtists(const QueryOptions& opt) {
  return GetAll("artist", opt);
}

QStringList LibraryBackend::GetAllArtistsWithAlbums(const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT artist");
  query.AddCompilationRequirement(false);
  query.AddWhere("album", "", "!=");

  QMutexLocker l(db_->Mutex());
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


SongList LibraryBackend::GetSongsByAlbum(const QString& album, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.AddCompilationRequirement(false);
  query.AddWhere("album", album);
  return ExecLibraryQuery(&query);
}

SongList LibraryBackend::GetSongs(const QString& artist, const QString& album, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.AddCompilationRequirement(false);
  query.AddWhere("artist", artist);
  query.AddWhere("album", album);
  return ExecLibraryQuery(&query);
}

SongList LibraryBackend::ExecLibraryQuery(LibraryQuery* query) {
  query->SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);
  QMutexLocker l(db_->Mutex());
  if (!ExecQuery(query)) return SongList();

  SongList ret;
  while (query->Next()) {
    Song song;
    song.InitFromQuery(*query, true);
    ret << song;
  }
  return ret;
}

Song LibraryBackend::GetSongById(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  return GetSongById(id, db);
}

SongList LibraryBackend::GetSongsById(const QList<int>& ids) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QStringList str_ids;
  foreach (int id, ids) {
    str_ids << QString::number(id);
  }

  return GetSongsById(str_ids, db);
}

SongList LibraryBackend::GetSongsById(const QStringList& ids) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  return GetSongsById(ids, db);
}

SongList LibraryBackend::GetSongsByForeignId(
    const QStringList& ids, const QString& table, const QString& column) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QString in = ids.join(",");

  QSqlQuery q(QString("SELECT %2.ROWID, " + Song::kColumnSpec + ", %2.%3"
                      " FROM %2, %1"
                      " WHERE %2.%3 IN (%4) AND %1.ROWID = %2.ROWID AND unavailable = 0")
              .arg(songs_table_, table, column, in), db);
  q.exec();
  if (db_->CheckErrors(q)) return SongList();

  QVector<Song> ret(ids.count());
  while (q.next()) {
    const QString foreign_id = q.value(Song::kColumns.count() + 1).toString();
    const int index = ids.indexOf(foreign_id);
    if (index == -1)
      continue;

    ret[index].InitFromQuery(q, true);
  }
  return ret.toList();
}

Song LibraryBackend::GetSongById(int id, QSqlDatabase& db) {
  SongList list = GetSongsById(QStringList() << QString::number(id), db);
  if (list.isEmpty())
    return Song();
  return list.first();
}

SongList LibraryBackend::GetSongsById(const QStringList& ids, QSqlDatabase& db) {
  QString in = ids.join(",");

  QSqlQuery q(QString("SELECT ROWID, " + Song::kColumnSpec + " FROM %1"
                      " WHERE ROWID IN (%2)").arg(songs_table_, in), db);
  q.exec();
  if (db_->CheckErrors(q)) return SongList();

  SongList ret;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q, true);
    ret << song;
  }
  return ret;
}

Song LibraryBackend::GetSongByUrl(const QUrl& url, qint64 beginning) {
  LibraryQuery query;
  query.SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);
  query.AddWhere("filename", url.toEncoded());
  query.AddWhere("beginning", beginning);

  Song song;
  if (ExecQuery(&query) && query.Next()) {
    song.InitFromQuery(query, true);
  }
  return song;
}

SongList LibraryBackend::GetSongsByUrl(const QUrl& url) {
  LibraryQuery query;
  query.SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);
  query.AddWhere("filename", url.toEncoded());

  SongList songlist;
  if (ExecQuery(&query)) {
    while(query.Next()) {
      Song song;
      song.InitFromQuery(query, true);

      songlist << song;
    }
  }
  return songlist;
}

LibraryBackend::AlbumList LibraryBackend::GetCompilationAlbums(const QueryOptions& opt) {
  return GetAlbums(QString(), true, opt);
}

SongList LibraryBackend::GetCompilationSongs(const QString& album, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);
  query.AddCompilationRequirement(true);
  query.AddWhere("album", album);

  QMutexLocker l(db_->Mutex());
  if (!ExecQuery(&query)) return SongList();

  SongList ret;
  while (query.Next()) {
    Song song;
    song.InitFromQuery(query, true);
    ret << song;
  }
  return ret;
}

void LibraryBackend::UpdateCompilations() {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  // Look for albums that have songs by more than one 'effective album artist' in the same
  // directory

  QSqlQuery q(QString("SELECT effective_albumartist, album, filename, sampler "
    "FROM %1 WHERE unavailable = 0 ORDER BY album").arg(songs_table_), db);
  q.exec();
  if (db_->CheckErrors(q)) return;

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
    int last_separator = filename.lastIndexOf('/');
    if (last_separator == -1)
      continue;

    CompilationInfo& info = compilation_info[album];
    info.artists.insert(artist);
    info.directories.insert(filename.left(last_separator));
    if (sampler) info.has_samplers = true;
    else         info.has_not_samplers = true;
  }

  // Now mark the songs that we think are in compilations
  QSqlQuery update(QString("UPDATE %1"
                           " SET sampler = :sampler,"
                           "     effective_compilation = ((compilation OR :sampler OR forced_compilation_on) AND NOT forced_compilation_off) + 0"
                           " WHERE album = :album AND unavailable = 0").arg(songs_table_), db);
  QSqlQuery find_songs(QString("SELECT ROWID, " + Song::kColumnSpec + " FROM %1"
                               " WHERE album = :album AND sampler = :sampler AND unavailable = 0")
                       .arg(songs_table_), db);

  SongList deleted_songs;
  SongList added_songs;

  ScopedTransaction transaction(&db);

  QMap<QString, CompilationInfo>::const_iterator it = compilation_info.constBegin();
  for ( ; it != compilation_info.constEnd() ; ++it) {
    const CompilationInfo& info = it.value();
    QString album(it.key());

    // If there were more 'effective album artists' than there were directories for this album,
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
    song.InitFromQuery(find_songs, true);
    deleted_songs << song;
    song.set_sampler(true);
    added_songs << song;
  }

  // Mark this album
  update.bindValue(":sampler", sampler);
  update.bindValue(":album", album);
  update.exec();
  db_->CheckErrors(update);
}

LibraryBackend::AlbumList LibraryBackend::GetAlbums(const QString& artist,
                                                    bool compilation,
                                                    const QueryOptions& opt) {
  AlbumList ret;

  LibraryQuery query(opt);
  query.SetColumnSpec("album, artist, compilation, sampler, art_automatic, "
                      "art_manual, filename");
  query.SetOrderBy("album");

  if (compilation) {
    query.AddCompilationRequirement(true);
  } else if (!artist.isNull()) {
    query.AddCompilationRequirement(false);
    query.AddWhere("artist", artist);
  }

  QMutexLocker l(db_->Mutex());
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
    info.first_url = QUrl::fromEncoded(query.Value(6).toByteArray());

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
  query.SetColumnSpec("art_automatic, art_manual, filename");
  query.AddWhere("artist", artist);
  query.AddWhere("album", album);

  QMutexLocker l(db_->Mutex());
  if (!ExecQuery(&query)) return ret;

  if (query.Next()) {
    ret.art_automatic = query.Value(0).toString();
    ret.art_manual = query.Value(1).toString();
    ret.first_url = QUrl::fromEncoded(query.Value(2).toByteArray());
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
  QMutexLocker l(db_->Mutex());
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
    song.InitFromQuery(query, true);
    deleted_songs << song;
  }


  // Update the songs
  QString sql(QString("UPDATE %1 SET art_manual = :art"
                      " WHERE album = :album AND unavailable = 0").arg(songs_table_));
  if (!artist.isNull())
    sql += " AND artist = :artist";

  QSqlQuery q(sql, db);
  q.bindValue(":art", art);
  q.bindValue(":album", album);
  if (!artist.isNull())
    q.bindValue(":artist", artist);

  q.exec();
  db_->CheckErrors(q);


  // Now get the updated songs
  if (!ExecQuery(&query)) return;

  SongList added_songs;
  while (query.Next()) {
    Song song;
    song.InitFromQuery(query, true);
    added_songs << song;
  }

  if (!added_songs.isEmpty() || !deleted_songs.isEmpty()) {
    emit SongsDeleted(deleted_songs);
    emit SongsDiscovered(added_songs);
  }
}

void LibraryBackend::ForceCompilation(const QString& album, const QList<QString>& artists, bool on) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  SongList deleted_songs, added_songs;

  foreach(const QString &artist, artists) {
    // Get the songs before they're updated
    LibraryQuery query;
    query.SetColumnSpec("ROWID, " + Song::kColumnSpec);
    query.AddWhere("album", album);
    if (!artist.isNull())
      query.AddWhere("artist", artist);

    if (!ExecQuery(&query)) return;

    while (query.Next()) {
      Song song;
      song.InitFromQuery(query, true);
      deleted_songs << song;
    }

    // Update the songs
    QString sql(QString("UPDATE %1 SET forced_compilation_on = :forced_compilation_on,"
                        "              forced_compilation_off = :forced_compilation_off,"
                        "              effective_compilation = ((compilation OR sampler OR :forced_compilation_on) AND NOT :forced_compilation_off) + 0"
                        " WHERE album = :album AND unavailable = 0").arg(songs_table_));
    if (!artist.isEmpty())
      sql += " AND artist = :artist";

    QSqlQuery q(sql, db);
    q.bindValue(":forced_compilation_on", on ? 1 : 0);
    q.bindValue(":forced_compilation_off", on ? 0 : 1);
    q.bindValue(":album", album);
    if (!artist.isEmpty())
      q.bindValue(":artist", artist);

    q.exec();
    db_->CheckErrors(q);

    // Now get the updated songs
    if (!ExecQuery(&query)) return;

    while (query.Next()) {
      Song song;
      song.InitFromQuery(query, true);
      added_songs << song;
    }
  }

  if (!added_songs.isEmpty() || !deleted_songs.isEmpty()) {
    emit SongsDeleted(deleted_songs);
    emit SongsDiscovered(added_songs);
  }
}

bool LibraryBackend::ExecQuery(LibraryQuery *q) {
  return !db_->CheckErrors(q->Exec(db_->Connect(), songs_table_, fts_table_));
}

SongList LibraryBackend::FindSongs(const smart_playlists::Search& search) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  // Build the query
  QString sql = search.ToSql(songs_table());

  // Run the query
  SongList ret;
  QSqlQuery query(sql, db);
  query.exec();
  if (db_->CheckErrors(query))
    return ret;

  // Read the results
  while (query.next()) {
    Song song;
    song.InitFromQuery(query, true);
    ret << song;
  }
  return ret;
}

SongList LibraryBackend::GetAllSongs() {
  // Get all the songs!
  return FindSongs(smart_playlists::Search(
      smart_playlists::Search::Type_All, smart_playlists::Search::TermList(),
      smart_playlists::Search::Sort_FieldAsc, smart_playlists::SearchTerm::Field_Artist, -1));
}

void LibraryBackend::IncrementPlayCount(int id) {
  if (id == -1)
    return;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("UPDATE %1 SET playcount = playcount + 1,"
                      "              lastplayed = :now,"
                      "              score = " + QString(kNewScoreSql).arg("1.0") +
                      " WHERE ROWID = :id").arg(songs_table_), db);
  q.bindValue(":now", QDateTime::currentDateTime().toTime_t());
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q))
    return;

  Song new_song = GetSongById(id, db);
  emit SongsStatisticsChanged(SongList() << new_song);
}

void LibraryBackend::IncrementSkipCount(int id, float progress) {
  if (id == -1)
    return;
  progress = qBound(0.0f, progress, 1.0f);

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("UPDATE %1 SET skipcount = skipcount + 1,"
                      "              score = " + QString(kNewScoreSql).arg(progress) +
                      " WHERE ROWID = :id").arg(songs_table_), db);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q))
    return;

  Song new_song = GetSongById(id, db);
  emit SongsStatisticsChanged(SongList() << new_song);
}

void LibraryBackend::ResetStatistics(int id) {
  if (id == -1)
    return;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString(
      "UPDATE %1 SET playcount = 0, skipcount = 0,"
      "              lastplayed = -1, score = 0"
      " WHERE ROWID = :id").arg(songs_table_), db);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q))
    return;

  Song new_song = GetSongById(id, db);
  emit SongsStatisticsChanged(SongList() << new_song);
}

void LibraryBackend::UpdateSongRating(int id, float rating) {
  if (id == -1)
    return;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(QString("UPDATE %1 SET rating = :rating"
                      " WHERE ROWID = :id").arg(songs_table_), db);
  q.bindValue(":rating", rating);
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q))
    return;

  Song new_song = GetSongById(id, db);
  emit SongsRatingChanged(SongList() << new_song);
}

void LibraryBackend::DeleteAll() {
  {
    QMutexLocker l(db_->Mutex());
    QSqlDatabase db(db_->Connect());
    ScopedTransaction t(&db);

    QSqlQuery q("DELETE FROM " + songs_table_, db);
    q.exec();
    if (db_->CheckErrors(q))
      return;

    q = QSqlQuery("DELETE FROM " + fts_table_, db);
    q.exec();
    if (db_->CheckErrors(q))
      return;

    t.Commit();
  }

  emit DatabaseReset();
}

void LibraryBackend::ReloadSettingsAsync() {
  QMetaObject::invokeMethod(this, "ReloadSettings", Qt::QueuedConnection);
}

void LibraryBackend::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Statistics
  {
    bool save_statistics_in_file = s.value("save_statistics_in_file", false).toBool();
    // Compare with previous value to know if we should connect, disconnect or nothing
    if (save_statistics_in_file_ && !save_statistics_in_file) {
      disconnect(this, SIGNAL(SongsStatisticsChanged(SongList)),
          TagReaderClient::Instance(), SLOT(UpdateSongsStatistics(SongList)));
    } else if (!save_statistics_in_file_ && save_statistics_in_file) {
      connect(this, SIGNAL(SongsStatisticsChanged(SongList)),
          TagReaderClient::Instance(), SLOT(UpdateSongsStatistics(SongList)));
    }
    // Save old value
    save_statistics_in_file_ = save_statistics_in_file;
  }

  // Rating
  {
    bool save_ratings_in_file = s.value("save_ratings_in_file", false).toBool();
    // Compare with previous value to know if we should connect, disconnect or nothing
    if (save_ratings_in_file_ && !save_ratings_in_file) {
      disconnect(this, SIGNAL(SongsRatingChanged(SongList)),
          TagReaderClient::Instance(), SLOT(UpdateSongsRating(SongList)));
    } else if (!save_ratings_in_file_ && save_ratings_in_file) {
      connect(this, SIGNAL(SongsRatingChanged(SongList)),
          TagReaderClient::Instance(), SLOT(UpdateSongsRating(SongList)));
    }
    // Save old value
    save_ratings_in_file_ = save_ratings_in_file;
  }
}
