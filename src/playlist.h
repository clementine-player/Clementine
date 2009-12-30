#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractItemModel>
#include <QList>

#include "playlistitem.h"
#include "song.h"
#include "radioitem.h"

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
    Column_Disc,
    Column_Year,
    Column_Genre,

    Column_BPM,
    Column_Bitrate,
    Column_Samplerate,
    Column_Filename,
    Column_Filesize,

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
  int current_index() const;
  int next_index() const;
  int previous_index() const;
  bool stop_after_current() const;

  PlaylistItem* item_at(int index) const { return items_[index]; }
  PlaylistItem* current_item() const;

  PlaylistItem::Options current_item_options() const;
  Song current_item_metadata() const;

  // Scrobbling
  int scrobble_point() const { return scrobble_point_; }
  bool has_scrobbled() const { return has_scrobbled_; }
  void set_scrobbled(bool v) { has_scrobbled_ = v; }

  // Changing the playlist
  QModelIndex InsertItems(const QList<PlaylistItem*>& items, int after = -1);
  QModelIndex InsertSongs(const SongList& items, int after = -1);
  QModelIndex InsertRadioStations(const QList<RadioItem*>& items, int after = -1);
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
  void set_current_index(int index);
  void Paused();
  void Playing();
  void Stopped();
  void IgnoreSorting(bool value) { ignore_sorting_ = value; }

  void ClearStreamMetadata();
  void SetStreamMetadata(const QUrl& url, const Song& song);

 private:
  void SetCurrentIsPaused(bool paused);
  void UpdateScrobblePoint();

 private:
  QList<PlaylistItem*> items_;

  QPersistentModelIndex current_item_;
  QPersistentModelIndex stop_after_;
  bool current_is_paused_;

  int scrobble_point_;
  bool has_scrobbled_;

  // Hack to stop QTreeView::setModel sorting the playlist
  bool ignore_sorting_;
};

#endif // PLAYLIST_H
