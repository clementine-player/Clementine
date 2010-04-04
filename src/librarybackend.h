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

#ifndef LIBRARYBACKEND_H
#define LIBRARYBACKEND_H

#include <QObject>
#include <QSqlError>
#include <QSqlDatabase>
#include <QMutex>
#include <QSet>

#include "directory.h"
#include "song.h"
#include "libraryquery.h"

#include <sqlite3.h>

#include "gtest/gtest_prod.h"

class LibraryBackendInterface : public QObject {
  Q_OBJECT

 public:
  LibraryBackendInterface(QObject* parent = 0);

  struct Album {
    Album() {}
    Album(const QString& _artist, const QString& _album_name,
          const QString& _art_automatic, const QString& _art_manual)
            : artist(_artist), album_name(_album_name),
              art_automatic(_art_automatic), art_manual(_art_manual) {}

    QString artist;
    QString album_name;

    QString art_automatic;
    QString art_manual;
  };
  typedef QList<Album> AlbumList;

  virtual void Stop() {};

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  virtual void LoadDirectoriesAsync() = 0;

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  virtual void UpdateTotalSongCountAsync() = 0;

  virtual SongList FindSongsInDirectory(int id) = 0;
  virtual SubdirectoryList SubdirsInDirectory(int id) = 0;

  virtual QStringList GetAllArtists(const QueryOptions& opt = QueryOptions()) = 0;
  virtual SongList GetSongs(const QString& artist, const QString& album, const QueryOptions& opt = QueryOptions()) = 0;

  virtual bool HasCompilations(const QueryOptions& opt = QueryOptions()) = 0;
  virtual SongList GetCompilationSongs(const QString& album, const QueryOptions& opt = QueryOptions()) = 0;

  virtual AlbumList GetAllAlbums(const QueryOptions& opt = QueryOptions()) = 0;
  virtual AlbumList GetAlbumsByArtist(const QString& artist, const QueryOptions& opt = QueryOptions()) = 0;
  virtual AlbumList GetCompilationAlbums(const QueryOptions& opt = QueryOptions()) = 0;

  virtual void UpdateManualAlbumArtAsync(const QString& artist, const QString& album, const QString& art) = 0;
  virtual Album GetAlbumArt(const QString& artist, const QString& album) = 0;

  virtual Song GetSongById(int id) = 0;

  virtual void AddDirectory(const QString& path) = 0;
  virtual void RemoveDirectory(const Directory& dir) = 0;

  virtual void UpdateCompilationsAsync() = 0;

  virtual bool ExecQuery(LibraryQuery* q) = 0;

 public slots:
  virtual void LoadDirectories() = 0;
  virtual void UpdateTotalSongCount() = 0;
  virtual void AddOrUpdateSongs(const SongList& songs) = 0;
  virtual void UpdateMTimesOnly(const SongList& songs) = 0;
  virtual void DeleteSongs(const SongList& songs) = 0;
  virtual void AddOrUpdateSubdirs(const SubdirectoryList& subdirs) = 0;
  virtual void UpdateCompilations() = 0;
  virtual void UpdateManualAlbumArt(const QString& artist, const QString& album, const QString& art) = 0;
  virtual void ForceCompilation(const QString& artist, const QString& album, bool on) = 0;

 signals:
  void Error(const QString& message);

  void DirectoryDiscovered(const Directory& dir, const SubdirectoryList& subdirs);
  void DirectoryDeleted(const Directory& dir);

  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);

  void TotalSongCountUpdated(int total);
};


class LibraryBackend : public LibraryBackendInterface {
  Q_OBJECT

 public:
  LibraryBackend(QObject* parent = 0, const QString& database_name = QString());

  static const int kSchemaVersion;

  // This actually refers to the location of the sqlite database
  static QString DefaultDatabaseDirectory();

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  void LoadDirectoriesAsync();

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  void UpdateTotalSongCountAsync();

  SongList FindSongsInDirectory(int id);
  SubdirectoryList SubdirsInDirectory(int id);

  QStringList GetAllArtists(const QueryOptions& opt = QueryOptions());
  SongList GetSongs(const QString& artist, const QString& album, const QueryOptions& opt = QueryOptions());

  bool HasCompilations(const QueryOptions& opt = QueryOptions());
  SongList GetCompilationSongs(const QString& album, const QueryOptions& opt = QueryOptions());

  AlbumList GetAllAlbums(const QueryOptions& opt = QueryOptions());
  AlbumList GetAlbumsByArtist(const QString& artist, const QueryOptions& opt = QueryOptions());
  AlbumList GetCompilationAlbums(const QueryOptions& opt = QueryOptions());

  void UpdateManualAlbumArtAsync(const QString& artist, const QString& album, const QString& art);
  Album GetAlbumArt(const QString& artist, const QString& album);

  Song GetSongById(int id);

  void AddDirectory(const QString& path);
  void RemoveDirectory(const Directory& dir);

  void UpdateCompilationsAsync();

  bool ExecQuery(LibraryQuery* q);

 public slots:
  void LoadDirectories();
  void UpdateTotalSongCount();
  void AddOrUpdateSongs(const SongList& songs);
  void UpdateMTimesOnly(const SongList& songs);
  void DeleteSongs(const SongList& songs);
  void AddOrUpdateSubdirs(const SubdirectoryList& subdirs);
  void UpdateCompilations();
  void UpdateManualAlbumArt(const QString& artist, const QString& album, const QString& art);
  void ForceCompilation(const QString& artist, const QString& album, bool on);

 private:
  struct CompilationInfo {
    CompilationInfo() : has_samplers(false), has_not_samplers(false) {}

    QSet<QString> artists;
    QSet<QString> directories;

    bool has_samplers;
    bool has_not_samplers;
  };

  QSqlDatabase Connect();
  void UpdateDatabaseSchema(int version, QSqlDatabase& db);
  bool CheckErrors(const QSqlError& error);

  void UpdateCompilations(QSqlQuery& find_songs, QSqlQuery& update,
                          SongList& deleted_songs, SongList& added_songs,
                          const QString& album, int sampler);
  AlbumList GetAlbums(const QString& artist, bool compilation = false,
                      const QueryOptions& opt = QueryOptions());
  SubdirectoryList SubdirsInDirectory(int id, QSqlDatabase& db);

 private:
  static const char* kDatabaseName;

  QString directory_;
  QMutex connect_mutex_;

  // Used by tests
  QString injected_database_name_;


  uint query_hash_;
  QStringList query_cache_;

  FRIEND_TEST(LibraryBackendTest, LikeWorksWithAllAscii);
  FRIEND_TEST(LibraryBackendTest, LikeWorksWithUnicode);
  FRIEND_TEST(LibraryBackendTest, LikeAsciiCaseInsensitive);
  FRIEND_TEST(LibraryBackendTest, LikeUnicodeCaseInsensitive);
  FRIEND_TEST(LibraryBackendTest, LikePerformance);
  FRIEND_TEST(LibraryBackendTest, LikeCacheInvalidated);
  FRIEND_TEST(LibraryBackendTest, LikeQuerySplit);

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

  static bool sStaticInitDone;
  static bool sLoadedSqliteSymbols;
};

#endif // LIBRARYBACKEND_H
