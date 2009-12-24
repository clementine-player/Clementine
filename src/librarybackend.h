#ifndef LIBRARYBACKEND_H
#define LIBRARYBACKEND_H

#include <QObject>
#include <QSqlError>
#include <QSqlDatabase>
#include <QMutex>

#include "directory.h"
#include "song.h"

struct QueryOptions;

class LibraryBackend : public QObject {
  Q_OBJECT

 public:
  LibraryBackend(QObject* parent = 0);

  // This actually refers to the location of the sqlite database
  static QString DefaultDirectory();

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  void LoadDirectoriesAsync();

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  void UpdateTotalSongCountAsync();

  SongList FindSongsInDirectory(int id);

  QStringList GetAllArtists(const QueryOptions& opt);
  QStringList GetAlbumsByArtist(const QueryOptions& opt, const QString& artist);
  SongList GetSongs(const QueryOptions& opt, const QString& artist, const QString& album);

  bool HasCompilations(const QueryOptions& opt);
  QStringList GetCompilationAlbums(const QueryOptions& opt);
  SongList GetCompilationSongs(const QueryOptions& opt, const QString& album);

  Song GetSongById(int id);

  void AddDirectory(const QString& path);
  void RemoveDirectory(const Directory& dir);

 public slots:
  void LoadDirectories();
  void UpdateTotalSongCount();
  void AddOrUpdateSongs(const SongList& songs);
  void UpdateMTimesOnly(const SongList& songs);
  void DeleteSongs(const SongList& songs);

 signals:
  void Error(const QString& message);

  void DirectoriesDiscovered(const DirectoryList& directories);
  void DirectoriesDeleted(const DirectoryList& directories);

  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);

  void TotalSongCountUpdated(int total);

 private:
  QSqlDatabase Connect();
  bool CheckErrors(const QSqlError& error);

 private:
  static const char* kDatabaseName;
  QString directory_;
  QMutex connect_mutex_;
};

#endif // LIBRARYBACKEND_H
