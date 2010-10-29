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
#include <QSet>

#include "directory.h"
#include "libraryquery.h"
#include "core/song.h"

#include <boost/shared_ptr.hpp>

class Database;
class SmartPlaylistSearch;

class LibraryBackendInterface : public QObject {
 public:
  LibraryBackendInterface(QObject* parent = 0) : QObject(parent) {}
  virtual ~LibraryBackendInterface() {}

  struct Album {
    Album() {}
    Album(const QString& _artist, const QString& _album_name,
          const QString& _art_automatic, const QString& _art_manual,
          const QString& _first_filename)
            : artist(_artist), album_name(_album_name),
              art_automatic(_art_automatic), art_manual(_art_manual),
              first_filename(_first_filename) {}

    QString artist;
    QString album_name;

    QString art_automatic;
    QString art_manual;
    QString first_filename;
  };
  typedef QList<Album> AlbumList;

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  virtual void LoadDirectoriesAsync() = 0;

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  virtual void UpdateTotalSongCountAsync() = 0;

  virtual SongList FindSongsInDirectory(int id) = 0;
  virtual SubdirectoryList SubdirsInDirectory(int id) = 0;
  virtual DirectoryList GetAllDirectories() = 0;
  virtual void ChangeDirPath(int id, const QString& new_path) = 0;

  virtual QStringList GetAllArtists(const QueryOptions& opt = QueryOptions()) = 0;
  virtual QStringList GetAllArtistsWithAlbums(const QueryOptions& opt = QueryOptions()) = 0;
  virtual SongList GetSongs(
      const QString& artist, const QString& album, const QueryOptions& opt = QueryOptions()) = 0;

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

  virtual bool ExecQuery(LibraryQuery* q) = 0;
};

class LibraryBackend : public LibraryBackendInterface {
  Q_OBJECT

 public:
  Q_INVOKABLE LibraryBackend(QObject* parent = 0);
  void Init(boost::shared_ptr<Database> db, const QString& songs_table,
            const QString& dirs_table, const QString& subdirs_table,
            const QString& fts_table);

  boost::shared_ptr<Database> db() const { return db_; }

  QString songs_table() const { return songs_table_; }
  QString dirs_table() const { return dirs_table_; }
  QString subdirs_table() const { return subdirs_table_; }

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  void LoadDirectoriesAsync();

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  void UpdateTotalSongCountAsync();

  SongList FindSongsInDirectory(int id);
  SubdirectoryList SubdirsInDirectory(int id);
  DirectoryList GetAllDirectories();
  void ChangeDirPath(int id, const QString& new_path);

  QStringList GetAllArtists(const QueryOptions& opt = QueryOptions());
  QStringList GetAllArtistsWithAlbums(const QueryOptions& opt = QueryOptions());
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

  bool ExecQuery(LibraryQuery* q);
  SongList FindSongs(const SmartPlaylistSearch& search);

  void IncrementPlayCountAsync(int id);
  void IncrementSkipCountAsync(int id);
  void UpdateSongRatingAsync(int id, float rating);

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
  void IncrementPlayCount(int id);
  void IncrementSkipCount(int id);
  void UpdateSongRating(int id, float rating);

 signals:
  void DirectoryDiscovered(const Directory& dir, const SubdirectoryList& subdirs);
  void DirectoryDeleted(const Directory& dir);

  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);
  void SongsStatisticsChanged(const SongList& songs);

  void TotalSongCountUpdated(int total);

 private:
  struct CompilationInfo {
    CompilationInfo() : has_samplers(false), has_not_samplers(false) {}

    QSet<QString> artists;
    QSet<QString> directories;

    bool has_samplers;
    bool has_not_samplers;
  };

  void UpdateCompilations(QSqlQuery& find_songs, QSqlQuery& update,
                          SongList& deleted_songs, SongList& added_songs,
                          const QString& album, int sampler);
  AlbumList GetAlbums(const QString& artist, bool compilation = false,
                      const QueryOptions& opt = QueryOptions());
  SubdirectoryList SubdirsInDirectory(int id, QSqlDatabase& db);
  Song GetSongById(int id, QSqlDatabase& db);

 private:
  boost::shared_ptr<Database> db_;
  QString songs_table_;
  QString dirs_table_;
  QString subdirs_table_;
  QString fts_table_;
};

#endif // LIBRARYBACKEND_H
