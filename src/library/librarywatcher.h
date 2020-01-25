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

#ifndef LIBRARYWATCHER_H
#define LIBRARYWATCHER_H

#include "directory.h"
#include "core/song.h"

#include <QHash>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QStringList>

class QFileSystemWatcher;
class QTimer;

class CueParser;
class FileSystemWatcherInterface;
class LibraryBackend;
class TaskManager;

class LibraryWatcher : public QObject {
  Q_OBJECT

 public:
  LibraryWatcher(QObject* parent = nullptr);

  static const char* kSettingsGroup;

  void set_backend(LibraryBackend* backend) { backend_ = backend; }
  void set_task_manager(TaskManager* task_manager) {
    task_manager_ = task_manager;
  }
  void set_device_name(const QString& device_name) {
    device_name_ = device_name;
  }

  void IncrementalScanAsync();
  void FullScanAsync();
  void SetRescanPausedAsync(bool pause);
  void ReloadSettingsAsync();
  // This thread-safe method will cause a scan of this directory to cancel to
  // unblock the watcher thread. It will then invoke the DoRemoveDirectory on
  // the watcher's thread to complete the removal.
  void RemoveDirectory(const Directory& dir);

  void Stop() { watched_dirs_.StopAll(); }

 signals:
  void NewOrUpdatedSongs(const SongList& songs);
  void SongsMTimeUpdated(const SongList& songs);
  void SongsDeleted(const SongList& songs);
  void SongsReadded(const SongList& songs, bool unavailable = false);
  void SubdirsDiscovered(const SubdirectoryList& subdirs);
  void SubdirsMTimeUpdated(const SubdirectoryList& subdirs);
  void CompilationsNeedUpdating();

  void ScanStarted(int task_id);

  void Error(const QString& message);

 public slots:
  void ReloadSettings();
  void AddDirectory(const Directory& dir, const SubdirectoryList& subdirs);
  void SetRescanPaused(bool pause);

 private:
  class WatchedDir : public Directory {
   public:
    WatchedDir() : active_(true) {}
    WatchedDir(const Directory& dir) : Directory(dir), active_(true) {}

    bool active_;
  };

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
    ScanTransaction(LibraryWatcher* watcher,
                    const LibraryWatcher::WatchedDir& dir, bool incremental,
                    bool ignores_mtime = false);
    ~ScanTransaction();

    SongList FindSongsInSubdirectory(const QString& path);
    bool HasSeenSubdir(const QString& path);
    void SetKnownSubdirs(const SubdirectoryList& subdirs);
    SubdirectoryList GetImmediateSubdirs(const QString& path);
    SubdirectoryList GetAllSubdirs();

    void AddToProgress(int n = 1);
    void AddToProgressMax(int n);

    const Directory& dir() const { return dir_; }
    int dir_id() const { return dir_.id; }
    bool is_incremental() const { return incremental_; }
    bool ignores_mtime() const { return ignores_mtime_; }

    bool aborted() { return !dir_.active_; }

    SongList deleted_songs;
    SongList readded_songs;
    SongList new_songs;
    SongList touched_songs;
    SubdirectoryList new_subdirs;
    SubdirectoryList touched_subdirs;
    SubdirectoryList deleted_subdirs;

   private:
    ScanTransaction& operator=(const ScanTransaction&) { return *this; }

    int task_id_;
    int progress_;
    int progress_max_;

    const WatchedDir& dir_;
    // Incremental scan enters a directory only if it has changed since the
    // last scan.
    bool incremental_;
    // This type of scan updates every file in a folder that's
    // being scanned. Even if it detects the file hasn't changed since
    // the last scan. Also, since it's ignoring mtimes on folders too,
    // it will go as deep in the folder hierarchy as it's possible.
    bool ignores_mtime_;

    LibraryWatcher* watcher_;

    SongList cached_songs_;
    bool cached_songs_dirty_;

    SubdirectoryList known_subdirs_;
    bool known_subdirs_dirty_;
  };

 private slots:
  void DirectoryChanged(const QString& path);
  void IncrementalScanNow();
  void FullScanNow();
  void RescanPathsNow();
  void ScanSubdirectory(const QString& path, const Subdirectory& subdir,
                        ScanTransaction* t, bool force_noincremental = false);
  void DoRemoveDirectory(const Directory& dir);

 private:
  static bool FindSongByPath(const SongList& list, const QString& path,
                             Song* out);
  inline static QString NoExtensionPart(const QString& fileName);
  inline static QString ExtensionPart(const QString& fileName);
  inline static QString DirectoryPart(const QString& fileName);
  QString PickBestImage(const QStringList& images, ScanTransaction* t);
  QString ImageForSong(const QString& path,
                       QMap<QString, QStringList>* album_art,
                       ScanTransaction* t);
  void AddWatch(const Directory& dir, const QString& path);
  void RemoveWatch(const Directory& dir, const Subdirectory& subdir);
  uint GetMtimeForCue(const QString& cue_path);
  void PerformScan(bool incremental, bool ignore_mtimes);

  // Updates the sections of a cue associated and altered (according to mtime)
  // media file during a scan.
  void UpdateCueAssociatedSongs(const QString& file, const QString& path,
                                const QString& matching_cue,
                                const QString& image, ScanTransaction* t);
  // Updates a single non-cue associated and altered (according to mtime) song
  // during a scan.
  void UpdateNonCueAssociatedSong(const QString& file,
                                  const Song& matching_song,
                                  const QString& image, bool cue_deleted,
                                  ScanTransaction* t);
  // Updates a new song with some metadata taken from it's equivalent old
  // song (for example rating and score).
  void PreserveUserSetData(const QString& file, const QString& image,
                           const Song& matching_song, Song* out,
                           ScanTransaction* t);
  // Scans a single media file that's present on the disk but not yet in the
  // library.
  // It may result in a multiple files added to the library when the media file
  // has many sections (like a CUE related media file).
  SongList ScanNewFile(const QString& file, const QString& path,
                       const QString& matching_cue,
                       QSet<QString>* cues_processed);

 private:
  LibraryBackend* backend_;
  TaskManager* task_manager_;
  QString device_name_;

  FileSystemWatcherInterface* fs_watcher_;
  QHash<QString, Directory> subdir_mapping_;

  /* A list of words use to try to identify the (likely) best image
   * found in an directory to use as cover artwork.
   * e.g. using ["front", "cover"] would identify front.jpg and
   * exclude back.jpg.
   */
  QStringList best_image_filters_;

  bool scan_on_startup_;
  bool monitor_;

  // All methods of QMap are reentrant We should only need to worry about
  // syncronizing methods that remove directories on the watcher thread and
  // methods that modify directories called from other threads.
  class WatchList {
   public:
    // These may be called from a different thread.
    void StopAll();
    void Stop(const Directory& dir);

    // This should only be called on the watcher thread.
    void Remove(int id);

    void Add(const Directory& dir);

    QMap<int, WatchedDir> list_;

   private:
    QMutex mutex_;
  };

  WatchList watched_dirs_;
  QTimer* rescan_timer_;
  QMap<int, QStringList>
      rescan_queue_;  // dir id -> list of subdirs to be scanned
  bool rescan_paused_;

  int total_watches_;

  CueParser* cue_parser_;

  static QStringList sValidImages;
};

inline QString LibraryWatcher::NoExtensionPart(const QString& fileName) {
  return fileName.contains('.') ? fileName.section('.', 0, -2) : "";
}
// Thanks Amarok
inline QString LibraryWatcher::ExtensionPart(const QString& fileName) {
  return fileName.contains('.')
             ? fileName.mid(fileName.lastIndexOf('.') + 1).toLower()
             : "";
}
inline QString LibraryWatcher::DirectoryPart(const QString& fileName) {
  return fileName.section('/', 0, -2);
}

#endif  // LIBRARYWATCHER_H
