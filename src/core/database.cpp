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

#include "config.h"
#include "database.h"
#include "scopedtransaction.h"
#include "utilities.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibrary>
#include <QLibraryInfo>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QtDebug>
#include <QThread>
#include <QVariant>

const char* Database::kDatabaseFilename = "clementine.db";
const int Database::kSchemaVersion = 28;
const char* Database::kMagicAllSongsTables = "%allsongstables";

int Database::sNextConnectionId = 1;
QMutex Database::sNextConnectionIdMutex;

Database::Token::Token(const QString& token, int start, int end)
    : token(token),
      start_offset(start),
      end_offset(end) {
}

struct sqlite3_tokenizer_module {
  int iVersion;
  int (*xCreate)(
    int argc,                           /* Size of argv array */
    const char *const*argv,             /* Tokenizer argument strings */
    sqlite3_tokenizer **ppTokenizer     /* OUT: Created tokenizer */
  );

  int (*xDestroy)(sqlite3_tokenizer *pTokenizer);

  int (*xOpen)(
    sqlite3_tokenizer *pTokenizer,       /* Tokenizer object */
    const char *pInput, int nBytes,      /* Input buffer */
    sqlite3_tokenizer_cursor **ppCursor  /* OUT: Created tokenizer cursor */
  );

  int (*xClose)(sqlite3_tokenizer_cursor *pCursor);

  int (*xNext)(
    sqlite3_tokenizer_cursor *pCursor,   /* Tokenizer cursor */
    const char **ppToken, int *pnBytes,  /* OUT: Normalized text for token */
    int *piStartOffset,  /* OUT: Byte offset of token in input buffer */
    int *piEndOffset,    /* OUT: Byte offset of end of token in input buffer */
    int *piPosition      /* OUT: Number of tokens returned before this one */
  );
};

struct sqlite3_tokenizer {
  const sqlite3_tokenizer_module *pModule;  /* The module for this tokenizer */
  /* Tokenizer implementations will typically add additional fields */
};

struct sqlite3_tokenizer_cursor {
  sqlite3_tokenizer *pTokenizer;       /* Tokenizer for this cursor. */
  /* Tokenizer implementations will typically add additional fields */
};

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

sqlite3_tokenizer_module* Database::sFTSTokenizer = NULL;


int Database::FTSCreate(int argc, const char* const* argv, sqlite3_tokenizer** tokenizer) {
  *tokenizer = reinterpret_cast<sqlite3_tokenizer*>(new UnicodeTokenizer);

  return SQLITE_OK;
}

int Database::FTSDestroy(sqlite3_tokenizer* tokenizer) {
  UnicodeTokenizer* real_tokenizer = reinterpret_cast<UnicodeTokenizer*>(tokenizer);
  delete real_tokenizer;
  return SQLITE_OK;
}

int Database::FTSOpen(
    sqlite3_tokenizer* pTokenizer,
    const char* input,
    int bytes,
    sqlite3_tokenizer_cursor** cursor) {
  UnicodeTokenizerCursor* new_cursor = new UnicodeTokenizerCursor;
  new_cursor->pTokenizer = pTokenizer;
  new_cursor->position = 0;

  QString str = QString::fromUtf8(input, bytes).toLower();
  QChar* data = str.data();
  // Decompose and strip punctuation.
  QList<Token> tokens;
  QString token;
  int start_offset = 0;
  int offset = 0;
  for (int i = 0; i < str.length(); ++i) {
    QChar c = data[i];
    ushort unicode = c.unicode();
    if (unicode <= 0x007f) {
      offset += 1;
    } else if (unicode >= 0x0080 && unicode <= 0x07ff) {
      offset += 2;
    } else if (unicode >= 0x0800) {
      offset += 3;
    }
    // Unicode astral planes unsupported in Qt?
    /*else if (unicode >= 0x010000 && unicode <= 0x10ffff) {
      offset += 4;
    }*/

    if (!data[i].isLetterOrNumber()) {
      // Token finished.
      if (token.length() != 0) {
        tokens << Token(token, start_offset, offset - 1);
        start_offset = offset;
        token.clear();
      } else {
        ++start_offset;
      }
    } else {
      if (data[i].decompositionTag() != QChar::NoDecomposition) {
        token.push_back(data[i].decomposition()[0]);
      } else {
        token.push_back(data[i]);
      }
    }

    if (i == str.length() - 1) {
      if (token.length() != 0) {
        tokens << Token(token, start_offset, offset);
        token.clear();
      }
    }
  }

  new_cursor->tokens = tokens;
  *cursor = reinterpret_cast<sqlite3_tokenizer_cursor*>(new_cursor);

  return SQLITE_OK;
}

int Database::FTSClose(sqlite3_tokenizer_cursor* cursor) {
  UnicodeTokenizerCursor* real_cursor = reinterpret_cast<UnicodeTokenizerCursor*>(cursor);
  delete real_cursor;

  return SQLITE_OK;
}

int Database::FTSNext(
    sqlite3_tokenizer_cursor* cursor,
    const char** token,
    int* bytes,
    int* start_offset,
    int* end_offset,
    int* position) {
  UnicodeTokenizerCursor* real_cursor = reinterpret_cast<UnicodeTokenizerCursor*>(cursor);

  QList<Token> tokens = real_cursor->tokens;
  if (real_cursor->position >= tokens.size()) {
    return SQLITE_DONE;
  }

  Token t = tokens[real_cursor->position];
  QByteArray utf8 = t.token.toUtf8();
  *token = utf8.constData();
  *bytes = utf8.size();
  *start_offset = t.start_offset;
  *end_offset = t.end_offset;
  *position = real_cursor->position++;

  real_cursor->current_utf8 = utf8;

  return SQLITE_OK;
}


void Database::StaticInit() {
  if (sStaticInitDone) {
    return;
  }
  sStaticInitDone = true;

  sFTSTokenizer = new sqlite3_tokenizer_module;
  sFTSTokenizer->iVersion = 0;
  sFTSTokenizer->xCreate = &Database::FTSCreate;
  sFTSTokenizer->xDestroy = &Database::FTSDestroy;
  sFTSTokenizer->xOpen = &Database::FTSOpen;
  sFTSTokenizer->xNext = &Database::FTSNext;
  sFTSTokenizer->xClose = &Database::FTSClose;

#ifdef HAVE_STATIC_SQLITE
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
#else // HAVE_STATIC_SQLITE
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
  {
    QMutexLocker l(&sNextConnectionIdMutex);
    connection_id_ = sNextConnectionId ++;
  }

  directory_ = QDir::toNativeSeparators(
      Utilities::GetConfigPath(Utilities::Path_Root));

  attached_databases_["jamendo"] = AttachedDatabase(
        directory_ + "/jamendo.db", ":/schema/jamendo.sql");

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

  const QString connection_id =
      QString("%1_thread_%2").arg(connection_id_).arg(
        reinterpret_cast<quint64>(QThread::currentThread()));

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

  QSqlQuery set_fts_tokenizer("SELECT fts3_tokenizer(:name, :pointer)", db);
  set_fts_tokenizer.bindValue(":name", "unicode");
  set_fts_tokenizer.bindValue(":pointer", QByteArray(
      reinterpret_cast<const char*>(&sFTSTokenizer), sizeof(&sFTSTokenizer)));
  if (!set_fts_tokenizer.exec()) {
    qWarning() << "Couldn't register FTS3 tokenizer";
  }

  // We want Unicode aware LIKE clauses and FTS if possible.
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

  // Attach external databases
  foreach (const QString& key, attached_databases_.keys()) {
    QString filename = attached_databases_[key].filename_;

    if (!injected_database_name_.isNull())
      filename = injected_database_name_;

    // Attach the db
    QSqlQuery q("ATTACH DATABASE :filename AS :alias", db);
    q.bindValue(":filename", filename);
    q.bindValue(":alias", key);
    if (!q.exec()) {
      qFatal("Couldn't attach external database '%s'", key.toAscii().constData());
    }
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

  // We might have to initialise the schema in some attached databases now, if
  // they were deleted and don't match up with the main schema version.
  foreach (const QString& key, attached_databases_.keys()) {
    // Find out if there are any tables in this database
    QSqlQuery q(QString("SELECT ROWID FROM %1.sqlite_master"
                        " WHERE type='table'").arg(key), db);
    if (!q.exec() || !q.next()) {
      ScopedTransaction t(&db);
      ExecFromFile(attached_databases_[key].schema_, db);
      t.Commit();
    }
  }

  return db;
}

void Database::RecreateAttachedDb(const QString& database_name) {
  if (!attached_databases_.contains(database_name)) {
    qWarning() << "Attached database does not exist:" << database_name;
    return;
  }

  const QString filename = attached_databases_[database_name].filename_;

  QMutexLocker l(&mutex_);
  {
    QSqlDatabase db(Connect());

    QSqlQuery q("DETACH DATABASE :alias", db);
    q.bindValue(":alias", database_name);
    if (!q.exec()) {
      qWarning() << "Failed to detach database" << database_name;
      return;
    }

    if (!QFile::remove(filename)) {
      qWarning() << "Failed to remove file" << filename;
    }
  }

  // We can't just re-attach the database now because it needs to be done for
  // each thread.  Close all the database connections, so each thread will
  // re-attach it when they next connect.
  foreach (const QString& name, QSqlDatabase::connectionNames()) {
    QSqlDatabase::removeDatabase(name);
  }
}

void Database::UpdateDatabaseSchema(int version, QSqlDatabase &db) {
  QString filename;
  if (version == 0)
    filename = ":/schema/schema.sql";
  else
    filename = QString(":/schema/schema-%1.sql").arg(version);

  ScopedTransaction t(&db);
  ExecFromFile(filename, db);
  t.Commit();
}

void Database::ExecFromFile(const QString &filename, QSqlDatabase &db) {
  // Open and read the database schema
  QFile schema_file(filename);
  if (!schema_file.open(QIODevice::ReadOnly))
    qFatal("Couldn't open schema file %s", filename.toUtf8().constData());
  ExecCommands(QString::fromUtf8(schema_file.readAll()), db);
}

void Database::ExecCommands(const QString &schema, QSqlDatabase &db) {
  // Run each command
  QStringList commands(schema.split(";\n\n"));

  // We don't want this list to reflect possible DB schema changes
  // so we initialize it before executing any statements.
  QStringList tables = SongsTables(db);

  foreach (const QString& command, commands) {
    // There are now lots of "songs" tables that need to have the same schema:
    // songs, magnatune_songs, and device_*_songs.  We allow a magic value
    // in the schema files to update all songs tables at once.
    if (command.contains(kMagicAllSongsTables)) {
      foreach (const QString& table, tables) {
        qDebug() << "Updating" << table << "for" << kMagicAllSongsTables;
        QString new_command(command);
        new_command.replace(kMagicAllSongsTables, table);
        QSqlQuery query(db.exec(new_command));
        if (CheckErrors(query))
          qFatal("Unable to update music library database");
      }
    } else {
      QSqlQuery query(db.exec(command));
      if (CheckErrors(query))
        qFatal("Unable to update music library database");
    }
  }
}

QStringList Database::SongsTables(QSqlDatabase& db) const {
  QStringList ret;

  // look for the tables in the main db
  foreach (const QString& table, db.tables()) {
    if (table == "songs" || table.endsWith("_songs"))
      ret << table;
  }

  // look for the tables in attached dbs
  foreach (const QString& key, attached_databases_.keys()) {
    QSqlQuery q(QString("SELECT NAME FROM %1.sqlite_master"
                        " WHERE type='table' AND name='songs' OR name LIKE '%songs'").arg(key), db);
    if (q.exec()) {
      while(q.next()) {
        QString tab_name = key + "." + q.value(0).toString();
        ret << tab_name;
      }
    }
  }

  return ret;
}

bool Database::CheckErrors(const QSqlQuery& query) {
  QSqlError last_error = query.lastError();
  if (last_error.isValid()) {
    qDebug() << "db error: " << last_error;
    qDebug() << "faulty query: " << query.lastQuery();
    qDebug() << "bound values: " << query.boundValues();

    emit Error("LibraryBackend: " + last_error.text());
    return true;
  }

  return false;
}
