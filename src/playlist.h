#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractItemModel>
#include <QList>

#include "playlistitem.h"
#include "song.h"

class RadioService;

class Playlist : public QAbstractListModel {
  Q_OBJECT

 public:
  Playlist(QObject* parent = 0);
  ~Playlist();

  enum Column {
    Column_Title = 0,
    Column_Artist,
    Column_Album,
    Column_Length,
    Column_Track,

    ColumnCount
  };

  enum Role {
    Role_IsCurrent = Qt::UserRole + 1,
    Role_IsPaused,
    Role_StopAfter,
  };

  static const char* kRowsMimetype;
  static const char* kSettingsGroup;

  static bool CompareItems(int column, Qt::SortOrder order,
                           const PlaylistItem* a, const PlaylistItem* b);

  // Persistence
  void Save() const;
  void Restore();

  // Accessors
  int current_item() const;
  int next_item() const;
  int previous_item() const;
  bool stop_after_current() const;
  PlaylistItem* item_at(int index) const { return items_[index]; }
  PlaylistItem::Options current_item_options() const;

  // Changing the playlist
  QModelIndex InsertItems(const QList<PlaylistItem*>& items, int after = -1);
  QModelIndex InsertSongs(const SongList& items, int after = -1);
  QModelIndex InsertRadioStations(const QList<RadioService*>& services,
                                  const QList<QUrl>& urls,
                                  const QStringList& titles, int after = -1);
  QModelIndex InsertPaths(QList<QUrl> urls, int after = -1);
  void StopAfter(int row);

  // QAbstractListModel
  int rowCount(const QModelIndex& = QModelIndex()) const { return items_.count(); }
  int columnCount(const QModelIndex& = QModelIndex()) const { return ColumnCount; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QStringList mimeTypes() const;
  Qt::DropActions supportedDropActions() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
  void sort(int column, Qt::SortOrder order);
  bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

 public slots:
  void set_current_item(int index);
  void Paused();
  void Playing();
  void Stopped();
  void IgnoreSorting(bool value) { ignore_sorting_ = value; }

  void ClearStreamMetadata();
  void SetStreamMetadata(const QUrl& url, const Song& song);

 private:
  void SetCurrentIsPaused(bool paused);

 private:
  QList<PlaylistItem*> items_;

  QPersistentModelIndex current_item_;
  QPersistentModelIndex stop_after_;
  bool current_is_paused_;

  // Hack to stop QTreeView::setModel sorting the playlist
  bool ignore_sorting_;
};

#endif // PLAYLIST_H
