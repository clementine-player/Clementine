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

class LibraryBackend : public QObject {
  Q_OBJECT

 public:
  LibraryBackend(QObject* parent = 0, const QString& database_name = QString());

  struct Album {
    QString artist;
    QString album_name;

    QString art_automatic;
    QString art_manual;
  };
  typedef QList<Album> AlbumList;

  static const int kSchemaVersion;

  // This actually refers to the location of the sqlite database
  static QString DefaultDatabaseDirectory();

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  void LoadDirectoriesAsync();

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  void UpdateTotalSongCountAsync();

  SongList FindSongsInDirectory(int id);

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

 public slots:
  void LoadDirectories();
  void UpdateTotalSongCount();
  void AddOrUpdateSongs(const SongList& songs);
  void UpdateMTimesOnly(const SongList& songs);
  void DeleteSongs(const SongList& songs);
  void UpdateCompilations();
  void UpdateManualAlbumArt(const QString& artist, const QString& album, const QString& art);
  void ForceCompilation(const QString& artist, const QString& album, bool on);

 signals:
  void Error(const QString& message);

  void DirectoriesDiscovered(const DirectoryList& directories);
  void DirectoriesDeleted(const DirectoryList& directories);

  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);

  void TotalSongCountUpdated(int total);

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

 private:
  static const char* kDatabaseName;

  QString directory_;
  QMutex connect_mutex_;

  // Used by tests
  QString injected_database_name_;


  // Do static initialisation like loading sqlite functions.
  static bool StaticInit();
  // Custom LIKE() function for sqlite.
  static void SqliteLike(sqlite3_context* context, int argc, sqlite3_value** argv);
  typedef void (*Sqlite3CreateFunc) (
      sqlite3*, const char*, int, int, void*,
      void (*) (sqlite3_context*, int, sqlite3_value**),
      void (*) (sqlite3_context*, int, sqlite3_value**),
      void (*) (sqlite3_context*));

  // Sqlite3 functions. These will be loaded from the sqlite3 plugin.
  static Sqlite3CreateFunc _sqlite3_create_function;
  static int (*_sqlite3_value_type) (sqlite3_value*);
  static sqlite_int64 (*_sqlite3_value_int64) (sqlite3_value*);
  static uchar* (*_sqlite3_value_text) (sqlite3_value*);
  static void (*_sqlite3_result_int64) (sqlite3_context*, int);
};

#endif // LIBRARYBACKEND_H
