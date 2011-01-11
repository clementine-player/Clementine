class LibraryBackend : QObject {

%TypeHeaderCode
#include "library/librarybackend.h"
%End

public:
  struct Album {
    Album();
    Album(const QString& _artist, const QString& _album_name,
          const QString& _art_automatic, const QString& _art_manual,
          const QString& _first_filename);

    QString artist;
    QString album_name;

    QString art_automatic;
    QString art_manual;
    QString first_filename;
  };
  typedef QList<LibraryBackend::Album> AlbumList;

  QString songs_table() const;
  QString dirs_table() const;
  QString subdirs_table() const;

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  void LoadDirectoriesAsync();

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  void UpdateTotalSongCountAsync();

  SongList FindSongsInDirectory(int id);
  SubdirectoryList SubdirsInDirectory(int id);
  DirectoryList GetAllDirectories();
  void ChangeDirPath(int id, const QString& new_path);

  QStringList GetAll(const QString& column, const QueryOptions& opt = QueryOptions());
  QStringList GetAllArtists(const QueryOptions& opt = QueryOptions());
  QStringList GetAllArtistsWithAlbums(const QueryOptions& opt = QueryOptions());
  SongList GetSongs(const QString& artist, const QString& album, const QueryOptions& opt = QueryOptions());

  bool HasCompilations(const QueryOptions& opt = QueryOptions());
  SongList GetCompilationSongs(const QString& album, const QueryOptions& opt = QueryOptions());

  LibraryBackend::AlbumList GetAllAlbums(const QueryOptions& opt = QueryOptions());
  LibraryBackend::AlbumList GetAlbumsByArtist(const QString& artist, const QueryOptions& opt = QueryOptions());
  LibraryBackend::AlbumList GetCompilationAlbums(const QueryOptions& opt = QueryOptions());

  void UpdateManualAlbumArtAsync(const QString& artist, const QString& album, const QString& art);
  LibraryBackend::Album GetAlbumArt(const QString& artist, const QString& album);

  Song GetSongById(int id);
  SongList GetSongsById(const QList<int>& ids);
  SongList GetSongsById(const QStringList& ids);
  SongList GetSongsByForeignId(const QStringList& ids, const QString& table,
                               const QString& column);

  Song GetSongByFilename(const QString& filename, int beginning);

  void AddDirectory(const QString& path);
  void RemoveDirectory(const Directory& dir);

  bool ExecQuery(LibraryQuery* q);
  // SongList FindSongs(const smart_playlists::Search& search);

  void IncrementPlayCountAsync(int id);
  void IncrementSkipCountAsync(int id, float progress);
  void ResetStatisticsAsync(int id);
  void UpdateSongRatingAsync(int id, float rating);

  void DeleteAll();

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
  void IncrementSkipCount(int id, float progress);
  void ResetStatistics(int id);
  void UpdateSongRating(int id, float rating);

signals:
  void DirectoryDiscovered(const Directory& dir, const SubdirectoryList& subdirs);
  void DirectoryDeleted(const Directory& dir);

  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);
  void SongsStatisticsChanged(const SongList& songs);
  void DatabaseReset();

  void TotalSongCountUpdated(int total);

private:
  LibraryBackend();
};
