#include "librarybackend.h"
#include "libraryquery.h"

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QSettings>
#include <QtDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QThread>

const char* LibraryBackend::kDatabaseName = "clementine.db";
const int LibraryBackend::kSchemaVersion = 2;

LibraryBackend::LibraryBackend(QObject* parent)
  : QObject(parent)
{
  QSettings s;
  s.beginGroup("Library");
  directory_ = s.value("database_directory", DefaultDirectory()).toString();

  Connect();
}

QString LibraryBackend::DefaultDirectory() {
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
  db.setDatabaseName(directory_ + "/" + kDatabaseName);
  if (!db.open()) {
    emit Error("LibraryBackend: " + db.lastError().text());
    return db;
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
    qDebug() << "Updating database schema from" << schema_version << "to" << kSchemaVersion;
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

  qDebug() << "Applying database schema version" << version;

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

QStringList LibraryBackend::GetAllAlbums(const QueryOptions &opt) {
  return GetAlbumsByArtist(QString(), opt);
}

QStringList LibraryBackend::GetAlbumsByArtist(const QString& artist, const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT album");
  query.AddCompilationRequirement(false);

  if (!artist.isNull())
    query.AddWhere("artist", artist);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return QStringList();

  QStringList ret;
  while (q.next()) {
    ret << q.value(0).toString();
  }
  return ret;
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

QStringList LibraryBackend::GetCompilationAlbums(const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT album");
  query.AddCompilationRequirement(true);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return QStringList();

  QStringList ret;
  while (q.next()) {
    ret << q.value(0).toString();
  }
  return ret;
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
  QSqlQuery update("UPDATE songs SET sampler = :sampler WHERE album = :album", db);
  QSqlQuery find_songs("SELECT ROWID, " + QString(Song::kColumnSpec) + " FROM songs"
                       " WHERE album = :album AND sampler = :sampler", db);

  SongList updated_songs;

  db.transaction();

  QMap<QString, CompilationInfo>::const_iterator it = compilation_info.constBegin();
  for ( ; it != compilation_info.constEnd() ; ++it) {
    const CompilationInfo& info = it.value();
    QString album(it.key());

    // If there were more artists than there were directories for this album,
    // then it's a compilation

    if (info.artists.count() > info.directories.count()) {
      if (info.has_not_samplers)
        UpdateCompilations(find_songs, update, updated_songs, album, 1);
    } else {
      if (info.has_samplers)
        UpdateCompilations(find_songs, update, updated_songs, album, 0);
    }
  }

  db.commit();

  if (!updated_songs.isEmpty()) {
    emit SongsDeleted(updated_songs);
    emit SongsDiscovered(updated_songs);
  }
}

void LibraryBackend::UpdateCompilations(QSqlQuery& find_songs, QSqlQuery& update,
                                        SongList& updated_songs,
                                        const QString& album, int sampler) {
  // Get songs that were already in that album, so we can tell the model
  // they've been updated
  find_songs.bindValue(":album", album);
  find_songs.bindValue(":sampler", int(!sampler));
  find_songs.exec();
  while (find_songs.next()) {
    Song song;
    song.InitFromQuery(find_songs);
    song.set_sampler(true);
    updated_songs << song;
  }

  // Mark this album
  update.bindValue(":sampler", sampler);
  update.bindValue(":album", album);
  update.exec();
  CheckErrors(update.lastError());
}

QList<LibraryBackend::AlbumArtInfo>
    LibraryBackend::GetAlbumArtInfo(const QString& artist,
                                    const QueryOptions& opt) {
  QList<AlbumArtInfo> ret;
  LibraryQuery query(opt);
  query.SetColumnSpec("album, artist, compilation, sampler, art_automatic, art_manual");
  query.SetOrderBy("album");

  if (!artist.isNull())
    query.AddWhere("artist", artist);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return ret;

  QString last_album;
  while (q.next()) {
    if (q.value(0).toString() == last_album)
      continue;

    bool compilation = q.value(2).toBool() | q.value(3).toBool();

    AlbumArtInfo info;
    info.artist = compilation ? QString() : q.value(1).toString();
    info.album_name = q.value(0).toString();
    info.art_automatic = q.value(4).toString();
    info.art_manual = q.value(5).toString();
    ret << info;

    last_album = info.album_name;
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
