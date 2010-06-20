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

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMutex>
#include <QStringList>

#include <sqlite3.h>

#include "gtest/gtest_prod.h"

extern "C" {

struct sqlite3_tokenizer;
struct sqlite3_tokenizer_cursor;

struct sqlite3_tokenizer_module {

  /*
  ** Structure version. Should always be set to 0.
  */
  int iVersion;

  /*
  ** Create a new tokenizer. The values in the argv[] array are the
  ** arguments passed to the "tokenizer" clause of the CREATE VIRTUAL
  ** TABLE statement that created the fts3 table. For example, if
  ** the following SQL is executed:
  **
  **   CREATE .. USING fts3( ... , tokenizer <tokenizer-name> arg1 arg2)
  **
  ** then argc is set to 2, and the argv[] array contains pointers
  ** to the strings "arg1" and "arg2".
  **
  ** This method should return either SQLITE_OK (0), or an SQLite error 
  ** code. If SQLITE_OK is returned, then *ppTokenizer should be set
  ** to point at the newly created tokenizer structure. The generic
  ** sqlite3_tokenizer.pModule variable should not be initialised by
  ** this callback. The caller will do so.
  */
  int (*xCreate)(
    int argc,                           /* Size of argv array */
    const char *const*argv,             /* Tokenizer argument strings */
    sqlite3_tokenizer **ppTokenizer     /* OUT: Created tokenizer */
  );

  /*
  ** Destroy an existing tokenizer. The fts3 module calls this method
  ** exactly once for each successful call to xCreate().
  */
  int (*xDestroy)(sqlite3_tokenizer *pTokenizer);

  /*
  ** Create a tokenizer cursor to tokenize an input buffer. The caller
  ** is responsible for ensuring that the input buffer remains valid
  ** until the cursor is closed (using the xClose() method). 
  */
  int (*xOpen)(
    sqlite3_tokenizer *pTokenizer,       /* Tokenizer object */
    const char *pInput, int nBytes,      /* Input buffer */
    sqlite3_tokenizer_cursor **ppCursor  /* OUT: Created tokenizer cursor */
  );

  /*
  ** Destroy an existing tokenizer cursor. The fts3 module calls this 
  ** method exactly once for each successful call to xOpen().
  */
  int (*xClose)(sqlite3_tokenizer_cursor *pCursor);

  /*
  ** Retrieve the next token from the tokenizer cursor pCursor. This
  ** method should either return SQLITE_OK and set the values of the
  ** "OUT" variables identified below, or SQLITE_DONE to indicate that
  ** the end of the buffer has been reached, or an SQLite error code.
  **
  ** *ppToken should be set to point at a buffer containing the 
  ** normalized version of the token (i.e. after any case-folding and/or
  ** stemming has been performed). *pnBytes should be set to the length
  ** of this buffer in bytes. The input text that generated the token is
  ** identified by the byte offsets returned in *piStartOffset and
  ** *piEndOffset. *piStartOffset should be set to the index of the first
  ** byte of the token in the input buffer. *piEndOffset should be set
  ** to the index of the first byte just past the end of the token in
  ** the input buffer.
  **
  ** The buffer *ppToken is set to point at is managed by the tokenizer
  ** implementation. It is only required to be valid until the next call
  ** to xNext() or xClose(). 
  */
  /* TODO(shess) current implementation requires pInput to be
  ** nul-terminated.  This should either be fixed, or pInput/nBytes
  ** should be converted to zInput.
  */
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

}

class Database : public QObject {
  Q_OBJECT

 public:
  Database(QObject* parent = 0, const QString& database_name = QString());

  static const int kSchemaVersion;
  static const char* kDatabaseFilename;

  void Stop() {}

  QSqlDatabase Connect();
  bool CheckErrors(const QSqlError& error);
  QMutex* Mutex() { return &mutex_; }

 signals:
  void Error(const QString& message);

 private:
  void UpdateDatabaseSchema(int version, QSqlDatabase& db);

  QString directory_;
  QMutex connect_mutex_;
  QMutex mutex_;

  // Used by tests
  QString injected_database_name_;

  uint query_hash_;
  QStringList query_cache_;

  FRIEND_TEST(DatabaseTest, LikeWorksWithAllAscii);
  FRIEND_TEST(DatabaseTest, LikeWorksWithUnicode);
  FRIEND_TEST(DatabaseTest, LikeAsciiCaseInsensitive);
  FRIEND_TEST(DatabaseTest, LikeUnicodeCaseInsensitive);
  FRIEND_TEST(DatabaseTest, LikePerformance);
  FRIEND_TEST(DatabaseTest, LikeCacheInvalidated);
  FRIEND_TEST(DatabaseTest, LikeQuerySplit);
  FRIEND_TEST(DatabaseTest, LikeDecomposes);

  // Do static initialisation like loading sqlite functions.
  static void StaticInit();

  // Custom LIKE() function for sqlite.
  bool Like(const char* needle, const char* haystack);
  static void SqliteLike(sqlite3_context* context, int argc, sqlite3_value** argv);
  typedef int (*Sqlite3CreateFunc) (
      sqlite3*, const char*, int, int, void*,
      void (*) (sqlite3_context*, int, sqlite3_value**),
      void (*) (sqlite3_context*, int, sqlite3_value**),
      void (*) (sqlite3_context*));

  // Sqlite3 functions. These will be loaded from the sqlite3 plugin.
  static Sqlite3CreateFunc _sqlite3_create_function;
  static int (*_sqlite3_value_type) (sqlite3_value*);
  static sqlite_int64 (*_sqlite3_value_int64) (sqlite3_value*);
  static const uchar* (*_sqlite3_value_text) (sqlite3_value*);
  static void (*_sqlite3_result_int64) (sqlite3_context*, sqlite_int64);
  static void* (*_sqlite3_user_data) (sqlite3_context*);

  static int (*_sqlite3_prepare_v2) (sqlite3*, const char*, int, sqlite3_stmt**, const char**);
  static int (*_sqlite3_bind_text) (sqlite3_stmt*, int, const char*, int, void(*)(void*));
  static int (*_sqlite3_bind_blob) (sqlite3_stmt*, int, const void*, int, void(*)(void*));
  static int (*_sqlite3_step) (sqlite3_stmt*);
  static int (*_sqlite3_finalize) (sqlite3_stmt*);


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
};

class MemoryDatabase : public Database {
 public:
  MemoryDatabase(QObject* parent = 0) : Database(parent, ":memory:") {}
  ~MemoryDatabase() {
    // Make sure Qt doesn't reuse the same database
    QSqlDatabase::removeDatabase(Connect().connectionName());
  }
};

#endif // DATABASE_H
