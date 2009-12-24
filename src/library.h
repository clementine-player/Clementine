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

class LibraryItem;
class LibraryConfig;

class Library : public QAbstractItemModel {
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
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex & index) const;
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  bool hasChildren(const QModelIndex &parent) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;

 signals:
  void Error(const QString& message);
  void TotalSongCountUpdated(int count);

 public slots:
  void ShowConfig();

  void SetFilterAge(int age);
  void SetFilterText(const QString& text);

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
  void LazyPopulate(LibraryItem* item);

  LibraryItem* CreateCompilationArtistNode(bool signal);
  LibraryItem* CreateArtistNode(bool signal, const QString& name);
  LibraryItem* CreateAlbumNode(bool signal, const QString& name, LibraryItem* parent, bool compilation);
  LibraryItem* CreateSongNode(bool signal, const Song& song, LibraryItem* parent);

  QString PrettyArtist(QString artist) const;
  QString PrettyAlbum(QString album) const;

  QString SortTextForArtist(QString artist) const;
  QString SortTextForAlbum(QString album) const;

  LibraryItem* IndexToItem(const QModelIndex& index) const;
  QModelIndex ItemToIndex(LibraryItem* item) const;
  QVariant data(const LibraryItem* item, int role) const;

  bool CompareItems(const LibraryItem* a, const LibraryItem* b) const;

 private:
  EngineBase* engine_;
  BackgroundThread<LibraryBackend>* backend_;
  BackgroundThread<LibraryWatcher>* watcher_;

  int waiting_for_threads_;

  QueryOptions query_options_;

  LibraryItem* root_;
  QMap<int, LibraryItem*> song_nodes_;
  QMap<QString, LibraryItem*> artist_nodes_;
  QMap<QChar, LibraryItem*> divider_nodes_;
  LibraryItem* compilation_artist_node_;

  QIcon artist_icon_;
  QIcon album_icon_;

  LibraryConfig* config_;
};

#endif // LIBRARY_H
