#ifndef LIBRARY_H
#define LIBRARY_H

#include <QAbstractItemModel>
#include <QIcon>

#include "backgroundthread.h"
#include "librarybackend.h"
#include "librarywatcher.h"
#include "libraryquery.h"
#include "engine_fwd.h"
#include "song.h"
#include "libraryitem.h"
#include "simpletreemodel.h"

class LibraryConfig;

class Library : public SimpleTreeModel<LibraryItem> {
  Q_OBJECT

 public:
  Library(EngineBase* engine, QObject* parent = 0);
  ~Library();

  enum {
    Role_Type = Qt::UserRole + 1,
    Role_SortText,
    Role_Key,
  };

  void StartThreads();

  // Get information about the library
  void GetChildSongs(LibraryItem* item, QList<QUrl>* urls, SongList* songs) const;
  SongList GetChildSongs(const QModelIndex& index) const;

  // QAbstractItemModel
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool canFetchMore(const QModelIndex &parent) const;

 signals:
  void Error(const QString& message);
  void TotalSongCountUpdated(int count);

 public slots:
  void ShowConfig();

  void SetFilterAge(int age);
  void SetFilterText(const QString& text);

 protected:
  void LazyPopulate(LibraryItem* item);

 private slots:
  // From LibraryBackend
  void BackendInitialised();
  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);
  void Reset();

  // From LibraryWatcher
  void WatcherInitialised();

 private:
  void Initialise();

  LibraryItem* CreateCompilationArtistNode(bool signal);
  LibraryItem* CreateArtistNode(bool signal, const QString& name);
  LibraryItem* CreateAlbumNode(bool signal, const QString& name, LibraryItem* parent, bool compilation);
  LibraryItem* CreateSongNode(bool signal, const Song& song, LibraryItem* parent);

  QString PrettyArtist(QString artist) const;
  QString PrettyAlbum(QString album) const;

  QString SortTextForArtist(QString artist) const;
  QString SortTextForAlbum(QString album) const;

  QVariant data(const LibraryItem* item, int role) const;

  bool CompareItems(const LibraryItem* a, const LibraryItem* b) const;

 private:
  EngineBase* engine_;
  BackgroundThread<LibraryBackend>* backend_;
  BackgroundThread<LibraryWatcher>* watcher_;

  int waiting_for_threads_;

  QueryOptions query_options_;

  QMap<int, LibraryItem*> song_nodes_;
  QMap<QString, LibraryItem*> artist_nodes_;
  QMap<QChar, LibraryItem*> divider_nodes_;
  LibraryItem* compilation_artist_node_;

  QIcon artist_icon_;
  QIcon album_icon_;

  LibraryConfig* config_;
};

#endif // LIBRARY_H
