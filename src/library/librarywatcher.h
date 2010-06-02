/* This file iss part of Clementine.

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

#ifndef LIBRARYWATCHER_H
#define LIBRARYWATCHER_H

#include "directory.h"
#include "core/song.h"

#include <QObject>
#include <QStringList>
#include <QMap>

class QFileSystemWatcher;
class QTimer;

class LibraryBackend;

class LibraryWatcher : public QObject {
  Q_OBJECT

 public:
  LibraryWatcher(QObject* parent = 0);

  static const char* kSettingsGroup;

  void SetBackend(LibraryBackend* backend) { backend_ = backend; }
  void IncrementalScanAsync();

  void Stop() { stop_requested_ = true; }

 signals:
  void NewOrUpdatedSongs(const SongList& songs);
  void SongsMTimeUpdated(const SongList& songs);
  void SongsDeleted(const SongList& songs);
  void SubdirsDiscovered(const SubdirectoryList& subdirs);
  void SubdirsMTimeUpdated(const SubdirectoryList& subdirs);
  void CompilationsNeedUpdating();

  void ScanStarted();
  void ScanFinished();

 public slots:
  void ReloadSettings();
  void AddDirectory(const Directory& dir, const SubdirectoryList& subdirs);
  void RemoveDirectory(const Directory& dir);

 private:
  // This class encapsulates a full or partial scan of a directory.
  // Each directory has one or more subdirectories, and any number of
  // subdirectories can be scanned during one transaction.  ScanSubdirectory()
  // adds its results to the members of this transaction class, and they are
  // "committed" through calls to the LibraryBackend in the transaction's dtor.
  // The transaction also caches the list of songs in this directory according
  // to the library.  Multiple calls to FindSongsInSubdirectory during one
  // transaction will only result in one call to
  // LibraryBackend::FindSongsInDirectory.
  class ScanTransaction {
   public:
    ScanTransaction(LibraryWatcher* watcher, int dir, bool incremental);
    ~ScanTransaction();

    SongList FindSongsInSubdirectory(const QString& path);
    bool HasSeenSubdir(const QString& path);
    void SetKnownSubdirs(const SubdirectoryList& subdirs);
    SubdirectoryList GetImmediateSubdirs(const QString& path);
    SubdirectoryList GetAllSubdirs();

    int dir() const { return dir_; }
    bool is_incremental() const { return incremental_; }

    SongList deleted_songs;
    SongList new_songs;
    SongList touched_songs;
    SubdirectoryList new_subdirs;
    SubdirectoryList touched_subdirs;

   private:
    ScanTransaction(const ScanTransaction&) {}
    ScanTransaction& operator =(const ScanTransaction&) { return *this; }

    int dir_;
    bool incremental_;
    LibraryWatcher* watcher_;

    SongList cached_songs_;
    bool cached_songs_dirty_;

    SubdirectoryList known_subdirs_;
    bool known_subdirs_dirty_;
  };

 private slots:
  void DirectoryChanged(const QString& path);
  void IncrementalScanNow();
  void RescanPathsNow();
  void ScanSubdirectory(const QString& path, const Subdirectory& subdir,
                        ScanTransaction* t, bool force_noincremental = false);

 private:
  static bool FindSongByPath(const SongList& list, const QString& path, Song* out);
  inline static QString ExtensionPart( const QString &fileName );
  inline static QString DirectoryPart( const QString &fileName );
  static QString PickBestImage(const QStringList& images);
  static QString ImageForSong(const QString& path, QMap<QString, QStringList>& album_art);
  void AddWatch(QFileSystemWatcher* w, const QString& path);

 private:
  // One of these gets stored for each Directory we're watching
  struct DirData {
    Directory dir;
    QFileSystemWatcher* watcher;
  };

  LibraryBackend* backend_;
  bool stop_requested_;
  bool scan_on_startup_;

  QMap<int, DirData> watched_dirs_;
  QTimer* rescan_timer_;
  QMap<int, QStringList> rescan_queue_; // dir id -> list of subdirs to be scanned

  int total_watches_;

  static QStringList sValidImages;
  static QStringList sValidPlaylists;

  #ifdef Q_OS_DARWIN
  static const int kMaxWatches = 100;
  #endif
};

// Thanks Amarok
inline QString LibraryWatcher::ExtensionPart( const QString &fileName ) {
  return fileName.contains( '.' ) ? fileName.mid( fileName.lastIndexOf('.') + 1 ).toLower() : "";
}
inline QString LibraryWatcher::DirectoryPart( const QString &fileName ) {
  return fileName.section( '/', 0, -2 );
}

#endif // LIBRARYWATCHER_H
