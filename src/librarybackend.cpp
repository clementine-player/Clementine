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
    QFile schema_file(":/schema.sql");
    schema_file.open(QIODevice::ReadOnly);
    QString schema(QString::fromUtf8(schema_file.readAll()));

    QStringList commands(schema.split(";\n\n"));
    foreach (const QString& command, commands) {
      QSqlQuery query(db.exec(command));
      if (CheckErrors(query.lastError())) return db;
    }
  }
  return db;
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
  query.AddWhere("compilation", 0);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return QStringList();

  QStringList ret;
  while (q.next()) {
    ret << q.value(0).toString();
  }
  return ret;
}

QStringList LibraryBackend::GetAlbumsByArtist(const QueryOptions& opt, const QString& artist) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT album");
  query.AddWhere("compilation", 0);
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

SongList LibraryBackend::GetSongs(const QueryOptions& opt, const QString& artist, const QString& album) {
  LibraryQuery query(opt);
  query.SetColumnSpec("ROWID, " + QString(Song::kColumnSpec));
  query.AddWhere("compilation", 0);
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
  query.AddWhere("compilation", 1);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return false;

  return q.next();
}

QStringList LibraryBackend::GetCompilationAlbums(const QueryOptions& opt) {
  LibraryQuery query(opt);
  query.SetColumnSpec("DISTINCT album");
  query.AddWhere("compilation", 1);

  QSqlQuery q(query.Query(Connect()));
  q.exec();
  if (CheckErrors(q.lastError())) return QStringList();

  QStringList ret;
  while (q.next()) {
    ret << q.value(0).toString();
  }
  return ret;
}

SongList LibraryBackend::GetCompilationSongs(const QueryOptions& opt, const QString& album) {
  LibraryQuery query(opt);
  query.SetColumnSpec("ROWID, " + QString(Song::kColumnSpec));
  query.AddWhere("compilation", 1);
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


