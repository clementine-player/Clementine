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

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QSettings>
#include <QtDebug>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QThread>
#include <QLibrary>
#include <QLibraryInfo>


const char* LibraryBackend::kDatabaseName = "clementine.db";
const int LibraryBackend::kSchemaVersion = 5;

int (*LibraryBackend::_sqlite3_create_function) (
    sqlite3*, const char*, int, int, void*,
    void (*) (sqlite3_context*, int, sqlite3_value**),
    void (*) (sqlite3_context*, int, sqlite3_value**),
    void (*) (sqlite3_context*)) = NULL;
int (*LibraryBackend::_sqlite3_value_type) (sqlite3_value*) = NULL;
sqlite_int64 (*LibraryBackend::_sqlite3_value_int64) (sqlite3_value*) = NULL;
const uchar* (*LibraryBackend::_sqlite3_value_text) (sqlite3_value*) = NULL;
void (*LibraryBackend::_sqlite3_result_int64) (sqlite3_context*, sqlite_int64) = NULL;
void* (*LibraryBackend::_sqlite3_user_data) (sqlite3_context*) = NULL;

bool LibraryBackend::sStaticInitDone = false;
bool LibraryBackend::sLoadedSqliteSymbols = false;


void LibraryBackend::StaticInit() {
  if (sStaticInitDone) {
    return;
  }
  sStaticInitDone = true;

#ifdef Q_OS_WIN32
  // We statically link libqsqlite.dll on windows so these symbols are already
  // available
  _sqlite3_create_function = sqlite3_create_function;
  _sqlite3_value_type = sqlite3_value_type;
  _sqlite3_value_int64 = sqlite3_value_int64;
  _sqlite3_value_text = sqlite3_value_text;
  _sqlite3_result_int64 = sqlite3_result_int64;
  _sqlite3_user_data = sqlite3_user_data;
  sLoadedSqliteSymbols = true;
#else // Q_OS_WIN32
  QString plugin_path = QLibraryInfo::location(QLibraryInfo::PluginsPath) +
                        "/sqldrivers/libqsqlite";

  QLibrary library(plugin_path);
  if (!library.load()) {
    qDebug() << "QLibrary::load() failed for " << plugin_path;
    return;
  }

  _sqlite3_create_function = reinterpret_cast<Sqlite3CreateFunc>(
      library.resolve("sqlite3_create_function"));
  _sqlite3_value_type = reinterpret_cast<int (*) (sqlite3_value*)>(
      library.resolve("sqlite3_value_type"));
  _sqlite3_value_int64 = reinterpret_cast<sqlite_int64 (*) (sqlite3_value*)>(
      library.resolve("sqlite3_value_int64"));
  _sqlite3_value_text = reinterpret_cast<const uchar* (*) (sqlite3_value*)>(
      library.resolve("sqlite3_value_text"));
  _sqlite3_result_int64 = reinterpret_cast<void (*) (sqlite3_context*, sqlite_int64)>(
      library.resolve("sqlite3_result_int64"));
  _sqlite3_user_data = reinterpret_cast<void* (*) (sqlite3_context*)>(
      library.resolve("sqlite3_user_data"));

  if (!_sqlite3_create_function ||
      !_sqlite3_value_type ||
      !_sqlite3_value_int64 ||
      !_sqlite3_value_text ||
      !_sqlite3_result_int64 ||
      !_sqlite3_user_data) {
    qDebug() << "Couldn't resolve sqlite symbols";
    sLoadedSqliteSymbols = false;
  } else {
    sLoadedSqliteSymbols = true;
  }
#endif
}

bool LibraryBackend::Like(const char* needle, const char* haystack) {
  uint hash = qHash(needle);
  if (!query_hash_ || hash != query_hash_) {
    // New query, parse and cache.
    query_cache_ = QString::fromUtf8(needle).section('%', 1, 1).split(' ');
    query_hash_ = hash;
  }
  QString b = QString::fromUtf8(haystack);
  foreach (const QString& query, query_cache_) {
    if (b.contains(query, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

// Custom LIKE(X, Y) function for sqlite3 that supports case insensitive unicode matching.
void LibraryBackend::SqliteLike(sqlite3_context* context, int argc, sqlite3_value** argv) {
  Q_ASSERT(argc == 2 || argc == 3);
  Q_ASSERT(_sqlite3_value_type(argv[0]) == _sqlite3_value_type(argv[1]));

  LibraryBackend* library = reinterpret_cast<LibraryBackend*>(_sqlite3_user_data(context));
  Q_ASSERT(library);

  switch (_sqlite3_value_type(argv[0])) {
    case SQLITE_INTEGER: {
      qint64 result = _sqlite3_value_int64(argv[0]) - _sqlite3_value_int64(argv[1]);
      _sqlite3_result_int64(context, result ? 0 : 1);
      break;
    }
    case SQLITE_TEXT: {
      const char* data_a = reinterpret_cast<const char*>(_sqlite3_value_text(argv[0]));
      const char* data_b = reinterpret_cast<const char*>(_sqlite3_value_text(argv[1]));
      _sqlite3_result_int64(context, library->Like(data_a, data_b) ? 1 : 0);
      break;
    }
  }
}

LibraryBackendInterface::LibraryBackendInterface(QObject *parent)
  : QObject(parent)
{
}

LibraryBackend::LibraryBackend(QObject* parent, const QString& database_name)
  : LibraryBackendInterface(parent),
    injected_database_name_(database_name),
    query_hash_(0)
{
  QSettings s;
  s.beginGroup("Library");
  directory_ = s.value("database_directory", DefaultDatabaseDirectory()).toString();

  Connect();
}

QString LibraryBackend::DefaultDatabaseDirectory() {
  QDir ret(QDir::homePath() + "/.config/" + QCoreApplication::organizationName());
  return QDir::toNativeSeparators(ret.path());
}

QSqlDatabase LibraryBackend::Connect() {
  QMutexLocker l(&connect_mutex_);

  // Create the directory if it doesn't exist
  if (!QFile::exists(directory_)) {
    QDir dir;
    if (!dir.mkpath(directory_)) {
    }
  }

  const QString connection_id("thread_" + QString::number(
      reinterpret_cast<quint64>(QThread::currentThread())));

  // Try to find an existing connection for this thread
  QSqlDatabase db = QSqlDatabase::database(connection_id);
  if (db.isOpen()) {
    return db;
  }

  db = QSqlDatabase::addDatabase("QSQLITE", connection_id);

  if (!injected_database_name_.isNull())
    db.setDatabaseName(injected_database_name_);
  else
    db.setDatabaseName(directory_ + "/" + kDatabaseName);

  if (!db.open()) {
    emit Error("LibraryBackend: " + db.lastError().text());
    return db;
  }

  // Find Sqlite3 functions in the Qt plugin.
  StaticInit();

  // We want Unicode aware LIKE clauses if possible
  if (sLoadedSqliteSymbols) {
    QVariant v = db.driver()->handle();
    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*") == 0) {
      sqlite3* handle = *static_cast<sqlite3**>(v.data());
      if (handle) {
        _sqlite3_create_function(
            handle,       // Sqlite3 handle.
            "LIKE",       // Function name (either override or new).
            2,            // Number of args.
            SQLITE_ANY,   // What types this function accepts.
            this,         // Custom data available via sqlite3_user_data().
            &LibraryBackend::SqliteLike,  // Our function :-)
            NULL, NULL);
      }
    }
  }

  if (db.tables().count() == 0) {
    // Set up initial schema
    UpdateDatabaseSchema(0, db);
  }

  // Get the database's schema version
  QSqlQuery q("SELECT version FROM schema_version", db);
  int schema_version = 0;
  if (q.next())
    schema_version = q.value(0).toInt();

  if (schema_version > kSchemaVersion) {
    qWarning() << "The database schema (version" << schema_version << ") is newer than I was expecting";
    return db;
  }
  if (schema_version < kSchemaVersion) {
    // Update the schema
    for (int v=schema_version+1 ; v<= kSchemaVersion ; ++v) {
      UpdateDatabaseSchema(v, db);
    }
  }

  return db;
}

void LibraryBackend::UpdateDatabaseSchema(int version, QSqlDatabase &db) {
  QString filename;
  if (version == 0)
    filename = ":/schema.sql";
  else
    filename = QString(":/schema-%1.sql").arg(version);

  // Open and read the database schema
  QFile schema_file(filename);
  if (!schema_file.open(QIODevice::ReadOnly))
    qFatal("Couldn't open schema file %s", filename.toUtf8().constData());
  QString schema(QString::fromUtf8(schema_file.readAll()));

  // Run each command
  QStringList commands(schema.split(";\n\n"));
  db.transaction();
  foreach (const QString& command, commands) {
    QSqlQuery query(db.exec(command));
    if (CheckErrors(query.lastError()))
      qFatal("Unable to update music library database");
  }
  db.commit();
}

bool LibraryBackend::CheckErrors(const QSqlError& error) {
  if (error.isValid()) {
    qDebug() << error;
    emit Error("LibraryBackend: " + error.text());
    return true;
  }
  return false;
}

void LibraryBackend::LoadDirectoriesAsync() {
  metaObject()->invokeMethod(this, "LoadDirectories", Qt::QueuedConnection);
}

void LibraryBackend::UpdateTotalSongCountAsync() {
  metaObject()->invokeMethod(this, "UpdateTotalSongCount", Qt::QueuedConnection);
}

void LibraryBackend::UpdateCompilationsAsync() {
  metaObject()->invokeMethod(this, "UpdateCompilations", Qt::QueuedConnection);
}

void LibraryBackend::LoadDirectories() {
  QSqlDatabase db(Connect());

  QSqlQuery q("SELECT ROWID, path"
              " FROM directories", db);
  q.exec();
  if (CheckErrors(q.lastError())) return;

  DirectoryList directories;
  while (q.next()) {
    Directory dir;
    dir.id = q.value(0).toInt();
    dir.path = q.value(1).toString();
    directories << dir;
  }
  emit DirectoriesDiscovered(directories);
}

void LibraryBackend::UpdateTotalSongCount() {
  QSqlDatabase db(Connect());

  QSqlQuery q("SELECT COUNT(*) FROM songs", db);
  q.exec();
  if (CheckErrors(q.lastError())) return;
  if (!q.next()) return;

  emit TotalSongCountUpdated(q.value(0).toInt());
}

void LibraryBackend::AddDirectory(const QString &path) {
  QSqlDatabase db(Connect());

  QSqlQuery q("INSERT INTO directories (path, subdirs)"
              " VALUES (:path, 1)", db);
  q.bindValue(":path", path);
  q.exec();
  if (CheckErrors(q.lastError())) return;

  Directory dir;
  dir.path = path;
  dir.id = q.lastInsertId().toInt();

  emit DirectoriesDiscovered(DirectoryList() << dir);
}

void LibraryBackend::RemoveDirectory(const Directory& dir) {
  QSqlDatabase db(Connect());

  // Remove songs first
  DeleteSongs(FindSongsInDirectory(dir.id));

  // Now remove the directory
  QSqlQuery q("DELETE FROM directories WHERE ROWID = :id", db);
  q.bindValue(":id", dir.id);
  q.exec();
  if (CheckErrors(q.lastError())) return;

  emit DirectoriesDeleted(DirectoryList() << dir);
}

SongList LibraryBackend::FindSongsInDirectory(int id) {
  QSqlDatabase db(Connect());

  QSqlQuery q("SELECT ROWID, " + QString(Song::kColumnSpec) +
              " FROM songs WHERE directory = :directory", db);
  q.bindValue(":directory", id);
  q.exec();
  if (CheckErrors(q.lastError())) return SongList();

  SongList ret;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q);
    ret << song;
  }
  return ret;
}

void LibraryBackend::AddOrUpdateSongs(const SongList& songs) {
  QSqlDatabase db(Connect());

  QSqlQuery check_dir(
      "SELECT ROWID FROM directories WHERE ROWID = :id", db);
  QSqlQuery add_song(
      "INSERT INTO songs (" + QString(Song::kColumnSpec) + ")"
      " VALUES (" + QString(Song::kBindSpec) + ")", db);
  QSqlQuery update_song(
      "UPDATE songs SET " + QString(Song::kUpdateSpec) +
      " WHERE ROWID = :id", db);

  db.transaction();

  SongList added_songs;
  SongList deleted_songs;

  foreach (const Song& song, songs) {
    // Do a sanity check first - make sure the song's directory still exists
    // This is to fix a possible race condition when a directory is removed
    // while LibraryWatcher is scanning it.
    check_dir.bindValue(":id", song.directory_id());
    check_dir.exec();
    if (CheckErrors(check_dir.lastError())) continue;

    if (!check_dir.next())
      continue; // Directory didn't exist


    if (song.id() == -1) {
      // Create
      song.BindToQuery(&add_song);
      add_song.exec();
      if (CheckErrors(add_song.lastError())) continue;

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
      if (CheckErrors(update_song.lastError())) continue;

      deleted_songs << old_song;
      added_songs << song;
    }
  }

  db.commit();

  if (!deleted_songs.isEmpty())
    emit SongsDeleted(deleted_songs);

  if (!added_songs.isEmpty())
    emit SongsDiscovered(added_songs);

  UpdateTotalSongCountAsync();
}

void LibraryBackend::UpdateMTimesOnly(const SongList& songs) {
  QSqlDatabase db(Connect());

  QSqlQuery q("UPDATE songs SET mtime = :mtime WHERE ROWID = :id", db);

  db.transaction();
  foreach (const Song& song, songs) {
    q.bindValue(":mtime", song.mtime());
    q.bindValue(":id", song.id());
    q.exec();
    CheckErrors(q.lastError());
  }
  db.commit();
}

void LibraryBackend::DeleteSongs(const SongList &songs) {
  QSqlDatabase db(Connect());

  QSqlQuery q("DELETE FROM songs WHERE ROWID = :id", db);

  db.transaction();
  foreach (const Song& song, songs) {
    q.bindValue(":id", song.id());
    q.exec();
    CheckErrors(q.lastError());
  }
  db.commit();

  emit SongsDeleted(songs);

  UpdateTotalSongCountAsync();
}

QStringList LibraryBackend::GetAllArtists(const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT artist");
  query.AddCompilationRequirement(false);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return QStringList();

  QStringList ret;
  while (q.next()) {
    ret << q.value(0).toString();
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
  query.SetColumnSpec("ROWID, " + QString(Song::kColumnSpec));
  query.AddCompilationRequirement(false);
  query.AddWhere("artist", artist);
  query.AddWhere("album", album);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return SongList();

  SongList ret;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q);
    ret << song;
  }
  return ret;
}

Song LibraryBackend::GetSongById(int id) {
  QSqlDatabase db(Connect());

  QSqlQuery q("SELECT ROWID, " + QString(Song::kColumnSpec) + " FROM songs"
              " WHERE ROWID = :id", db);
  q.bindValue(":id", id);
  q.exec();
  if (CheckErrors(q.lastError())) return Song();

  q.next();

  Song ret;
  ret.InitFromQuery(q);
  return ret;
}

bool LibraryBackend::HasCompilations(const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("ROWID");
  query.AddCompilationRequirement(true);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return false;

  return q.next();
}

LibraryBackend::AlbumList LibraryBackend::GetCompilationAlbums(const QueryOptions& opt) {
  return GetAlbums(QString(), true, opt);
}

SongList LibraryBackend::GetCompilationSongs(const QString& album, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("ROWID, " + QString(Song::kColumnSpec));
  query.AddCompilationRequirement(true);
  query.AddWhere("album", album);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return SongList();

  SongList ret;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q);
    ret << song;
  }
  return ret;
}

void LibraryBackend::UpdateCompilations() {
  QSqlDatabase db(Connect());

  // Look for albums that have songs by more than one artist in the same
  // directory

  QSqlQuery q("SELECT artist, album, filename, sampler FROM songs ORDER BY album", db);
  q.exec();
  if (CheckErrors(q.lastError())) return;

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
  QSqlQuery update("UPDATE songs"
                   " SET sampler = :sampler,"
                   "     effective_compilation = ((compilation OR :sampler OR forced_compilation_on) AND NOT forced_compilation_off) + 0"
                   " WHERE album = :album", db);
  QSqlQuery find_songs("SELECT ROWID, " + QString(Song::kColumnSpec) + " FROM songs"
                       " WHERE album = :album AND sampler = :sampler", db);

  SongList deleted_songs;
  SongList added_songs;

  db.transaction();

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

  db.commit();

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
  CheckErrors(update.lastError());
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

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return ret;

  QString last_album;
  QString last_artist;
  while (q.next()) {
    bool compilation = q.value(2).toBool() | q.value(3).toBool();

    Album info;
    info.artist = compilation ? QString() : q.value(1).toString();
    info.album_name = q.value(0).toString();
    info.art_automatic = q.value(4).toString();
    info.art_manual = q.value(5).toString();

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

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return ret;

  if (q.next()) {
    ret.art_automatic = q.value(0).toString();
    ret.art_manual = q.value(1).toString();
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
  QSqlDatabase db(Connect());

  QString sql("UPDATE songs SET art_manual = :art"
              " WHERE album = :album");
  if (!artist.isNull())
    sql += " AND artist = :artist";

  QSqlQuery q(sql, db);
  q.bindValue(":art", art);
  q.bindValue(":album", album);
  if (!artist.isNull())
    q.bindValue(":artist", artist);

  q.exec();
  CheckErrors(q.lastError());
}

void LibraryBackend::ForceCompilation(const QString& artist, const QString& album, bool on) {
  QSqlDatabase db(Connect());

  // Get the songs before they're updated
  LibraryQuery query;
  query.SetColumnSpec("ROWID, " + QString(Song::kColumnSpec));
  query.AddWhere("album", album);
  if (!artist.isNull())
    query.AddWhere("artist", artist);

  QSqlQuery q(query.Query(db));
  q.exec();
  CheckErrors(q.lastError());

  SongList deleted_songs;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q);
    deleted_songs << song;
  }

  // Update the songs
  QString sql("UPDATE songs SET forced_compilation_on = :forced_compilation_on,"
              "                 forced_compilation_off = :forced_compilation_off,"
              "                 effective_compilation = ((compilation OR sampler OR :forced_compilation_on) AND NOT :forced_compilation_off) + 0"
              " WHERE album = :album");
  if (!artist.isEmpty())
    sql += " AND artist = :artist";

  q = QSqlQuery(sql, db);
  q.bindValue(":forced_compilation_on", on ? 1 : 0);
  q.bindValue(":forced_compilation_off", on ? 0 : 1);
  q.bindValue(":album", album);
  if (!artist.isEmpty())
    q.bindValue(":artist", artist);

  q.exec();
  CheckErrors(q.lastError());

  // Now get the updated songs
  q = QSqlQuery(query.Query(db));
  q.exec();
  CheckErrors(q.lastError());

  SongList added_songs;
  while (q.next()) {
    Song song;
    song.InitFromQuery(q);
    added_songs << song;
  }

  if (!added_songs.isEmpty() || !deleted_songs.isEmpty()) {
    emit SongsDeleted(deleted_songs);
    emit SongsDiscovered(added_songs);
  }
}
