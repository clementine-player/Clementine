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

#include "database.h"

#include <QLibrary>
#include <QLibraryInfo>
#include <QDir>
#include <QCoreApplication>
#include <QtDebug>
#include <QThread>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QVariant>

const char* Database::kDatabaseFilename = "clementine.db";
const int Database::kSchemaVersion = 13;

int (*Database::_sqlite3_create_function) (
    sqlite3*, const char*, int, int, void*,
    void (*) (sqlite3_context*, int, sqlite3_value**),
    void (*) (sqlite3_context*, int, sqlite3_value**),
    void (*) (sqlite3_context*)) = NULL;
int (*Database::_sqlite3_value_type) (sqlite3_value*) = NULL;
sqlite_int64 (*Database::_sqlite3_value_int64) (sqlite3_value*) = NULL;
const uchar* (*Database::_sqlite3_value_text) (sqlite3_value*) = NULL;
void (*Database::_sqlite3_result_int64) (sqlite3_context*, sqlite_int64) = NULL;
void* (*Database::_sqlite3_user_data) (sqlite3_context*) = NULL;

bool Database::sStaticInitDone = false;
bool Database::sLoadedSqliteSymbols = false;


void Database::StaticInit() {
  if (sStaticInitDone) {
    return;
  }
  sStaticInitDone = true;

#ifndef Q_WS_X11
  // We statically link libqsqlite.dll on windows and mac so these symbols are already
  // available
  _sqlite3_create_function = sqlite3_create_function;
  _sqlite3_value_type = sqlite3_value_type;
  _sqlite3_value_int64 = sqlite3_value_int64;
  _sqlite3_value_text = sqlite3_value_text;
  _sqlite3_result_int64 = sqlite3_result_int64;
  _sqlite3_user_data = sqlite3_user_data;
  sLoadedSqliteSymbols = true;
  return;
#else // Q_WS_X11
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

bool Database::Like(const char* needle, const char* haystack) {
  uint hash = qHash(needle);
  if (!query_hash_ || hash != query_hash_) {
    // New query, parse and cache.
    query_cache_ = QString::fromUtf8(needle).section('%', 1, 1).split(' ');
    query_hash_ = hash;
  }

  // In place decompose string as it's faster :-)
  QString b = QString::fromUtf8(haystack);
  QChar* data = b.data();
  for (int i = 0; i < b.length(); ++i) {
    if (data[i].decompositionTag() != QChar::NoDecomposition) {
      data[i] = data[i].decomposition()[0];
    }
  }
  foreach (const QString& query, query_cache_) {
    if (!b.contains(query, Qt::CaseInsensitive)) {
      return false;
    }
  }
  return true;
}

// Custom LIKE(X, Y) function for sqlite3 that supports case insensitive unicode matching.
void Database::SqliteLike(sqlite3_context* context, int argc, sqlite3_value** argv) {
  Q_ASSERT(argc == 2 || argc == 3);

  if (_sqlite3_value_type(argv[0]) == SQLITE_NULL ||
      _sqlite3_value_type(argv[1]) == SQLITE_NULL) {
    _sqlite3_result_int64(context, 0);
    return;
  }

  Q_ASSERT(_sqlite3_value_type(argv[0]) == _sqlite3_value_type(argv[1]));

  Database* library = reinterpret_cast<Database*>(_sqlite3_user_data(context));
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

Database::Database(QObject* parent, const QString& database_name)
  : QObject(parent),
    mutex_(QMutex::Recursive),
    injected_database_name_(database_name),
    query_hash_(0)
{
  directory_ = QDir::toNativeSeparators(
      QDir::homePath() + "/.config/" + QCoreApplication::organizationName());

  QMutexLocker l(&mutex_);
  Connect();
}

QSqlDatabase Database::Connect() {
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
    db.setDatabaseName(directory_ + "/" + kDatabaseFilename);

  if (!db.open()) {
    emit Error("Database: " + db.lastError().text());
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
            &Database::SqliteLike,  // Our function :-)
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

void Database::UpdateDatabaseSchema(int version, QSqlDatabase &db) {
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

bool Database::CheckErrors(const QSqlError& error) {
  if (error.isValid()) {
    qDebug() << error;
    emit Error("LibraryBackend: " + error.text());
    return true;
  }
  return false;
}
