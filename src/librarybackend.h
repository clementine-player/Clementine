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

class LibraryBackend : public QObject {
  Q_OBJECT

 public:
  LibraryBackend(QObject* parent = 0);

  struct AlbumArtInfo {
    QString album_name;
    QString art_automatic;
    QString art_manual;
  };

  // This actually refers to the location of the sqlite database
  static QString DefaultDirectory();

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  void LoadDirectoriesAsync();

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  void UpdateTotalSongCountAsync();

  SongList FindSongsInDirectory(int id);

  QStringList GetAllArtists(const QueryOptions& opt = QueryOptions());
  QStringList GetAllAlbums(const QueryOptions& opt = QueryOptions());
  QStringList GetAlbumsByArtist(const QString& artist, const QueryOptions& opt = QueryOptions());
  SongList GetSongs(const QString& artist, const QString& album, const QueryOptions& opt = QueryOptions());

  bool HasCompilations(const QueryOptions& opt = QueryOptions());
  QStringList GetCompilationAlbums(const QueryOptions& opt = QueryOptions());
  SongList GetCompilationSongs(const QString& album, const QueryOptions& opt = QueryOptions());

  QList<AlbumArtInfo> GetAlbumArtInfo(const QString& artist = QString(), const QueryOptions& opt = QueryOptions());

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
                          SongList& updated_songs,
                          const QString& album, int sampler);

 private:
  static const char* kDatabaseName;
  static const int kSchemaVersion;

  QString directory_;
  QMutex connect_mutex_;
};

#endif // LIBRARYBACKEND_H
