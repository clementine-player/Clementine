#ifndef LIBRARYWATCHER_H
#define LIBRARYWATCHER_H

#include "directory.h"
#include "song.h"
#include "engine_fwd.h"

#include <QObject>
#include <QStringList>
#include <QMap>

#include <boost/shared_ptr.hpp>

class QFileSystemWatcher;
class QTimer;

class LibraryBackend;

class LibraryWatcher : public QObject {
  Q_OBJECT

 public:
  LibraryWatcher(QObject* parent = 0);

  void SetBackend(boost::shared_ptr<LibraryBackend> backend) { backend_ = backend; }
  void SetEngine(EngineBase* engine) { engine_ = engine; } // TODO: shared_ptr

 signals:
  void NewOrUpdatedSongs(const SongList& songs);
  void SongsMTimeUpdated(const SongList& songs);
  void SongsDeleted(const SongList& songs);

  void ScanStarted();
  void ScanFinished();

 public slots:
  void AddDirectories(const DirectoryList& directories);
  void RemoveDirectories(const DirectoryList& directories);

 private slots:
  void DirectoryChanged(const QString& path);
  void RescanPathsNow();
  void ScanDirectory(const QString& path);

 private:
  static bool FindSongByPath(const SongList& list, const QString& path, Song* out);

 private:
  EngineBase* engine_;
  boost::shared_ptr<LibraryBackend> backend_;

  QFileSystemWatcher* fs_watcher_;
  QTimer* rescan_timer_;

  QMap<QString, Directory> paths_watched_;
  QStringList paths_needing_rescan_;

  int total_watches_;

  #ifdef Q_OS_DARWIN
  static const int kMaxWatches = 200;
  #endif
};

#endif // LIBRARYWATCHER_H
