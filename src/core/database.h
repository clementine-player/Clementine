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

#ifndef DATABASE_H
#define DATABASE_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>

#include <sqlite3.h>

#include "gtest/gtest_prod.h"

extern "C" {

struct sqlite3_tokenizer;
struct sqlite3_tokenizer_cursor;
struct sqlite3_tokenizer_module;

}

class Application;

class Database : public QObject {
  Q_OBJECT

 public:
  Database(Application* app, QObject* parent = 0,
           const QString& database_name = QString());

  struct AttachedDatabase {
    AttachedDatabase() {}
    AttachedDatabase(const QString& filename, const QString& schema, bool is_temporary)
      : filename_(filename), schema_(schema), is_temporary_(is_temporary) {}

    QString filename_;
    QString schema_;
    bool is_temporary_;
  };

  static const int kSchemaVersion;
  static const char* kDatabaseFilename;
  static const char* kMagicAllSongsTables;

  QSqlDatabase Connect();
  bool CheckErrors(const QSqlQuery& query);
  QMutex* Mutex() { return &mutex_; }

  void RecreateAttachedDb(const QString& database_name);
  void ExecSchemaCommands(QSqlDatabase& db,
                          const QString& schema,
                          int schema_version,
                          bool in_transaction = false);

  int startup_schema_version() const { return startup_schema_version_; }
  int current_schema_version() const { return kSchemaVersion; }

  void AttachDatabase(const QString& database_name, const AttachedDatabase& database);
  void AttachDatabaseOnDbConnection(const QString& database_name,
                                    const AttachedDatabase& database,
                                    QSqlDatabase& db);
  void DetachDatabase(const QString& database_name);

 signals:
  void Error(const QString& message);

 public slots:
  void DoBackup();

 private:
  void UpdateMainSchema(QSqlDatabase* db);

  void ExecSchemaCommandsFromFile(QSqlDatabase& db,
                                  const QString& filename,
                                  int schema_version,
                                  bool in_transaction = false);
  void ExecSongTablesCommands(QSqlDatabase& db,
                              const QStringList& song_tables,
                              const QStringList& commands);

  void UpdateDatabaseSchema(int version, QSqlDatabase& db);
  void UrlEncodeFilenameColumn(const QString& table, QSqlDatabase& db);
  QStringList SongsTables(QSqlDatabase& db, int schema_version) const;
  bool IntegrityCheck(QSqlDatabase db);
  void BackupFile(const QString& filename);
  bool OpenDatabase(const QString& filename, sqlite3** connection) const;

  Application* app_;

  // Alias -> filename
  QMap<QString, AttachedDatabase> attached_databases_;

  QString directory_;
  QMutex connect_mutex_;
  QMutex mutex_;

  // This ID makes the QSqlDatabase name unique to the object as well as the
  // thread
  int connection_id_;

  static QMutex sNextConnectionIdMutex;
  static int sNextConnectionId;

  // Used by tests
  QString injected_database_name_;

  uint query_hash_;
  QStringList query_cache_;

  // This is the schema version of Clementine's DB from the app's last run.
  int startup_schema_version_;

  FRIEND_TEST(DatabaseTest, FTSOpenParsesSimpleInput);
  FRIEND_TEST(DatabaseTest, FTSOpenParsesUTF8Input);
  FRIEND_TEST(DatabaseTest, FTSOpenParsesMultipleTokens);
  FRIEND_TEST(DatabaseTest, FTSCursorWorks);
  FRIEND_TEST(DatabaseTest, FTSOpenLeavesCyrillicQueries);

  // Do static initialisation like loading sqlite functions.
  static void StaticInit();

  typedef int (*Sqlite3CreateFunc) (
      sqlite3*, const char*, int, int, void*,
      void (*) (sqlite3_context*, int, sqlite3_value**),
      void (*) (sqlite3_context*, int, sqlite3_value**),
      void (*) (sqlite3_context*));

  // Sqlite3 functions. These will be loaded from the sqlite3 plugin.
  static int (*_sqlite3_value_type) (sqlite3_value*);
  static sqlite_int64 (*_sqlite3_value_int64) (sqlite3_value*);
  static const uchar* (*_sqlite3_value_text) (sqlite3_value*);
  static void (*_sqlite3_result_int64) (sqlite3_context*, sqlite_int64);
  static void* (*_sqlite3_user_data) (sqlite3_context*);

  // These are necessary for SQLite backups.
  static int (*_sqlite3_open) (const char*, sqlite3**);
  static const char* (*_sqlite3_errmsg) (sqlite3*);
  static int (*_sqlite3_close) (sqlite3*);
  static sqlite3_backup* (*_sqlite3_backup_init) (
      sqlite3*, const char*, sqlite3*, const char*);
  static int (*_sqlite3_backup_step) (sqlite3_backup*, int);
  static int (*_sqlite3_backup_finish) (sqlite3_backup*);
  static int (*_sqlite3_backup_pagecount) (sqlite3_backup*);
  static int (*_sqlite3_backup_remaining) (sqlite3_backup*);

  static bool sStaticInitDone;
  static bool sLoadedSqliteSymbols;

  static sqlite3_tokenizer_module* sFTSTokenizer;

  static int FTSCreate(int argc, const char* const* argv, sqlite3_tokenizer** tokenizer);
  static int FTSDestroy(sqlite3_tokenizer* tokenizer);
  static int FTSOpen(sqlite3_tokenizer* tokenizer,
                     const char* input,
                     int bytes,
                     sqlite3_tokenizer_cursor** cursor);
  static int FTSClose(sqlite3_tokenizer_cursor* cursor);
  static int FTSNext(sqlite3_tokenizer_cursor* cursor,
                     const char** token,
                     int* bytes,
                     int* start_offset,
                     int* end_offset,
                     int* position);
  struct Token {
    Token(const QString& token, int start, int end);
    QString token;
    int start_offset;
    int end_offset;
  };

  // Based on sqlite3_tokenizer.
  struct UnicodeTokenizer {
    const sqlite3_tokenizer_module* pModule;
  };

  struct UnicodeTokenizerCursor {
    const sqlite3_tokenizer* pTokenizer;

    QList<Token> tokens;
    int position;
    QByteArray current_utf8;
  };
};

class MemoryDatabase : public Database {
 public:
  MemoryDatabase(Application* app, QObject* parent = 0)
    : Database(app, parent, ":memory:") {}
  ~MemoryDatabase() {
    // Make sure Qt doesn't reuse the same database
    QSqlDatabase::removeDatabase(Connect().connectionName());
  }
};

#endif // DATABASE_H
