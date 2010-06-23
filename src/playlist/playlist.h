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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractItemModel>
#include <QList>

#include <boost/shared_ptr.hpp>

#include "playlistitem.h"
#include "playlistsequence.h"
#include "core/song.h"
#include "radio/radioitem.h"

class RadioService;
class PlaylistBackend;
class PlaylistFilter;
class TaskManager;

class QSortFilterProxyModel;
class QUndoStack;

namespace PlaylistUndoCommands {
  class InsertItems;
  class RemoveItems;
  class MoveItems;
}

class Playlist : public QAbstractListModel {
  Q_OBJECT

  friend class PlaylistUndoCommands::InsertItems;
  friend class PlaylistUndoCommands::RemoveItems;
  friend class PlaylistUndoCommands::MoveItems;

 public:
  Playlist(PlaylistBackend* backend, TaskManager* task_manager, int id,
           QObject* parent = 0);
  ~Playlist();

  enum Column {
    Column_Title = 0,
    Column_Artist,
    Column_Album,
    Column_AlbumArtist,
    Column_Composer,
    Column_Length,
    Column_Track,
    Column_Disc,
    Column_Year,
    Column_Genre,

    Column_BPM,
    Column_Bitrate,
    Column_Samplerate,
    Column_Filename,
    Column_BaseFilename,
    Column_Filesize,
    Column_Filetype,
    Column_DateCreated,
    Column_DateModified,

    ColumnCount
  };

  enum Role {
    Role_IsCurrent = Qt::UserRole + 1,
    Role_IsPaused,
    Role_StopAfter,
  };

  static const char* kRowsMimetype;
  static const char* kPlayNowMimetype;

  static bool CompareItems(int column, Qt::SortOrder order,
                           boost::shared_ptr<PlaylistItem> a,
                           boost::shared_ptr<PlaylistItem> b);

  static QString column_name(Column column);
  static bool column_is_editable(Playlist::Column column);
  static bool set_column_value(Song& song, Column column, const QVariant& value);

  // Persistence
  void Save() const;
  void Restore();

  // Accessors
  QSortFilterProxyModel* proxy() const;

  int id() const { return id_; }
  int current_index() const;
  int last_played_index() const;
  int next_index() const;
  int previous_index() const;
  bool stop_after_current() const;

  const boost::shared_ptr<PlaylistItem>& item_at(int index) const { return items_[index]; }
  boost::shared_ptr<PlaylistItem> current_item() const { return current_item_; }

  PlaylistItem::Options current_item_options() const;
  Song current_item_metadata() const;

  PlaylistItemList library_items_by_id(int id) const;

  SongList GetAllSongs() const;
  quint64 GetTotalLength() const; // in seconds

  void set_sequence(PlaylistSequence* v);
  PlaylistSequence* sequence() const { return playlist_sequence_; }

  QUndoStack* undo_stack() const { return undo_stack_; }

  // Scrobbling
  int scrobble_point() const { return scrobble_point_; }
  bool has_scrobbled() const { return has_scrobbled_; }
  void set_scrobbled(bool v) { has_scrobbled_ = v; }

  // Changing the playlist
  QModelIndex InsertItems(const PlaylistItemList& items, int pos = -1);
  QModelIndex InsertLibraryItems(const SongList& items, int pos = -1);
  QModelIndex InsertMagnatuneItems(const SongList& items, int pos = -1);
  QModelIndex InsertSongs(const SongList& items, int pos = -1);
  QModelIndex InsertRadioStations(const QList<RadioItem*>& items, int pos = -1, bool play_now = false);
  void InsertUrls(const QList<QUrl>& urls, bool play_now, int pos = -1);
  void StopAfter(int row);
  void ReloadItems(const QList<int>& rows);

  // QAbstractListModel
  int rowCount(const QModelIndex& = QModelIndex()) const { return items_.count(); }
  int columnCount(const QModelIndex& = QModelIndex()) const { return ColumnCount; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
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

  void Clear();
  void Shuffle();

  void ShuffleModeChanged(PlaylistSequence::ShuffleMode mode);

 signals:
  void CurrentSongChanged(const Song& metadata);
  void EditingFinished(const QModelIndex& index);
  void PlayRequested(const QModelIndex& index);

  void PlaylistChanged();

  void LoadTracksError(const QString& message);

 private:
  void SetCurrentIsPaused(bool paused);
  void UpdateScrobblePoint();
  void ReshuffleIndices();
  int NextVirtualIndex(int i) const;
  int PreviousVirtualIndex(int i) const;
  bool FilterContainsVirtualIndex(int i) const;

  // Modify the playlist without changing the undo stack.  These are used by
  // our friends in PlaylistUndoCommands
  QModelIndex InsertItemsWithoutUndo(const PlaylistItemList& items, int pos);
  PlaylistItemList RemoveItemsWithoutUndo(int pos, int count);
  void MoveItemsWithoutUndo(const QList<int>& source_rows, int pos);
  void MoveItemsWithoutUndo(int start, const QList<int>& dest_rows);

 private:
  PlaylistFilter* proxy_;

  PlaylistBackend* backend_;
  TaskManager* task_manager_;
  int id_;

  PlaylistItemList items_;
  QList<int> virtual_items_; // Contains the indices into items_ in the order
                             // that they will be played.
  // A map of library ID to playlist item - for fast lookups when library
  // items change.
  QMultiMap<int, boost::shared_ptr<PlaylistItem> > library_items_by_id_;

  QPersistentModelIndex current_item_index_;
  QPersistentModelIndex last_played_item_index_;
  QPersistentModelIndex stop_after_;
  bool current_is_paused_;
  int current_virtual_index_;

  boost::shared_ptr<PlaylistItem> current_item_;

  bool is_shuffled_;

  int scrobble_point_;
  bool has_scrobbled_;

  PlaylistSequence* playlist_sequence_;

  // Hack to stop QTreeView::setModel sorting the playlist
  bool ignore_sorting_;

  QUndoStack* undo_stack_;
};

#endif // PLAYLIST_H
