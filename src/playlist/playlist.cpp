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

#include "playlist.h"
#include "playlistbackend.h"
#include "playlistfilter.h"
#include "playlistundocommands.h"
#include "queue.h"
#include "songloaderinserter.h"
#include "songmimedata.h"
#include "songplaylistitem.h"
#include "core/modelfuturewatcher.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "library/libraryplaylistitem.h"
#include "radio/jamendoplaylistitem.h"
#include "radio/jamendoservice.h"
#include "radio/magnatuneplaylistitem.h"
#include "radio/magnatuneservice.h"
#include "radio/radiomimedata.h"
#include "radio/radiomodel.h"
#include "radio/radioplaylistitem.h"
#include "radio/savedradio.h"
#include "smartplaylists/generator.h"
#include "smartplaylists/generatorinserter.h"
#include "smartplaylists/generatormimedata.h"

#include <QtDebug>
#include <QApplication>
#include <QMimeData>
#include <QBuffer>
#include <QFileInfo>
#include <QDirIterator>
#include <QUndoStack>
#include <QSortFilterProxyModel>

#include <boost/bind.hpp>
#include <algorithm>

using smart_playlists::Generator;
using smart_playlists::GeneratorInserter;
using smart_playlists::GeneratorPtr;

using boost::shared_ptr;

const char* Playlist::kRowsMimetype = "application/x-clementine-playlist-rows";
const char* Playlist::kPlayNowMimetype = "application/x-clementine-play-now";

Playlist::Playlist(PlaylistBackend* backend,
                   TaskManager* task_manager,
                   LibraryBackend* library,
                   int id,
                   QObject *parent)
  : QAbstractListModel(parent),
    is_loading_(false),
    proxy_(new PlaylistFilter(this)),
    queue_(new Queue(this)),
    backend_(backend),
    task_manager_(task_manager),
    library_(library),
    id_(id),
    current_is_paused_(false),
    current_virtual_index_(-1),
    is_shuffled_(false),
    scrobble_point_(-1),
    has_scrobbled_(false),
    playlist_sequence_(NULL),
    ignore_sorting_(false),
    undo_stack_(new QUndoStack(this))
{
  connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SIGNAL(PlaylistChanged()));
  connect(this, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SIGNAL(PlaylistChanged()));

  Restore();

  proxy_->setSourceModel(this);
  queue_->setSourceModel(this);

  connect(queue_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
          SLOT(TracksAboutToBeDequeued(QModelIndex,int,int)));
  connect(queue_, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(TracksDequeued()));

  connect(queue_, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          SLOT(TracksEnqueued(const QModelIndex&, int, int)));

  connect(queue_, SIGNAL(layoutChanged()), SLOT(QueueLayoutChanged()));

  column_alignments_[Column_Length]
      = column_alignments_[Column_Track]
      = column_alignments_[Column_Disc]
      = column_alignments_[Column_Year]
      = column_alignments_[Column_BPM]
      = column_alignments_[Column_Bitrate]
      = column_alignments_[Column_Samplerate]
      = column_alignments_[Column_Filesize]
      = column_alignments_[Column_PlayCount]
      = column_alignments_[Column_SkipCount]
      = (Qt::AlignRight | Qt::AlignVCenter);

  column_alignments_[Column_Score] = (Qt::AlignCenter);
}

Playlist::~Playlist() {
  items_.clear();
  library_items_by_id_.clear();
}

template<typename T>
static QModelIndex InsertSongItems(Playlist* playlist, const SongList& songs,
                                   int pos, bool enqueue = false) {
  PlaylistItemList items;
  foreach (const Song& song, songs) {
    items << PlaylistItemPtr(new T(song));
  }
  return playlist->InsertItems(items, pos, enqueue);
}

QVariant Playlist::headerData(int section, Qt::Orientation, int role) const {
  if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
    return QVariant();

  QString name = column_name((Playlist::Column)section);
  if(name.size())
    return name;

  return QVariant();
}

bool Playlist::column_is_editable(Playlist::Column column) {
  switch(column) {
    case Column_Title:
    case Column_Artist:
    case Column_Album:
    case Column_AlbumArtist:
    case Column_Composer:
    case Column_Track:
    case Column_Disc:
    case Column_Year:
    case Column_Genre:
    case Column_Score:
    case Column_Comment:
      return true;
    default:
      break;
  }
  return false;
}

bool Playlist::set_column_value(Song& song, Playlist::Column column,
                                const QVariant& value) {

  if (!song.IsEditable())
    return false;

  switch(column) {
    case Column_Title:
      song.set_title(value.toString());
      break;
    case Column_Artist:
      song.set_artist(value.toString());
      break;
    case Column_Album:
      song.set_album(value.toString());
      break;
    case Column_AlbumArtist:
      song.set_albumartist(value.toString());
      break;
    case Column_Composer:
      song.set_composer(value.toString());
      break;
    case Column_Track:
      song.set_track(value.toInt());
      break;
    case Column_Disc:
      song.set_disc(value.toInt());
      break;
    case Column_Year:
      song.set_year(value.toInt());
      break;
    case Column_Genre:
      song.set_genre(value.toString());
      break;
    case Column_Score:
      song.set_score(value.toInt());
      break;
    case Column_Comment:
      song.set_comment(value.toString());
      break;
    default:
      break;
  }
  return true;
}

QVariant Playlist::data(const QModelIndex& index, int role) const {
  switch (role) {
    case Role_IsCurrent:
      return current_item_index_.isValid() && index.row() == current_item_index_.row();

    case Role_IsPaused:
      return current_is_paused_;

    case Role_StopAfter:
      return stop_after_.isValid() && stop_after_.row() == index.row();

    case Role_QueuePosition:
      return queue_->PositionOf(index);

    case Role_CanSetRating:
      return index.column() == Column_Rating &&
             items_[index.row()]->IsLocalLibraryItem();

    case Qt::EditRole:
    case Qt::ToolTipRole:
    case Qt::DisplayRole: {
      PlaylistItemPtr item = items_[index.row()];
      Song song = item->Metadata();

      // Don't forget to change Playlist::CompareItems when adding new columns
      switch (index.column()) {
        case Column_Title:
          if (!song.title().isEmpty())
            return song.title();
          if (!song.basefilename().isEmpty())
            return song.basefilename();
          return song.filename();
        case Column_Artist:       return song.artist();
        case Column_Album:        return song.album();
        case Column_Length:       return song.length();
        case Column_Track:        return song.track();
        case Column_Disc:         return song.disc();
        case Column_Year:         return song.year();
        case Column_Genre:        return song.genre();
        case Column_AlbumArtist:  return song.albumartist();
        case Column_Composer:     return song.composer();

        case Column_Rating:       return song.rating();
        case Column_PlayCount:    return song.playcount();
        case Column_SkipCount:    return song.skipcount();
        case Column_LastPlayed:   return song.lastplayed();
        case Column_Score:        return song.score();

        case Column_BPM:          return song.bpm();
        case Column_Bitrate:      return song.bitrate();
        case Column_Samplerate:   return song.samplerate();
        case Column_Filename:     return song.filename();
        case Column_BaseFilename: return song.basefilename();
        case Column_Filesize:     return song.filesize();
        case Column_Filetype:     return song.filetype();
        case Column_DateModified: return song.mtime();
        case Column_DateCreated:  return song.ctime();

        case Column_Comment:
          if (role == Qt::DisplayRole)
            return song.comment().simplified();
          return song.comment();
      }
    }

    case Qt::TextAlignmentRole:
      return QVariant(column_alignments_.value(index.column(), (Qt::AlignLeft | Qt::AlignVCenter)));

    case Qt::ForegroundRole:
      if (items_[index.row()]->IsDynamicHistory())
        return QApplication::palette().brush(QPalette::Disabled, QPalette::Text);
      return QVariant();

    default:
      return QVariant();
  }
}

bool Playlist::setData(const QModelIndex &index, const QVariant &value, int) {
  int row = index.row();
  Song song = item_at(row)->Metadata();

  if (index.data() == value)
    return false;

  if(!set_column_value(song, (Column)index.column(), value))
    return false;

  if((Column)index.column() == Column_Score) {
    // The score is only saved in the database, not the file
    library_->AddOrUpdateSongs(SongList() << song);
    emit EditingFinished(index);
  } else {
    QFuture<bool> future = song.BackgroundSave();
    ModelFutureWatcher<bool>* watcher = new ModelFutureWatcher<bool>(index, this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), SLOT(SongSaveComplete()));
  }
  return true;
}

void Playlist::SongSaveComplete() {
  ModelFutureWatcher<bool>* watcher = static_cast<ModelFutureWatcher<bool>*>(sender());
  watcher->deleteLater();
  const QPersistentModelIndex& index = watcher->index();
  if (index.isValid()) {
    QFuture<void> future = item_at(index.row())->BackgroundReload();
    ModelFutureWatcher<void>* watcher = new ModelFutureWatcher<void>(index, this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), SLOT(ItemReloadComplete()));
  }
}

void Playlist::ItemReloadComplete() {
  ModelFutureWatcher<void>* watcher = static_cast<ModelFutureWatcher<void>*>(sender());
  watcher->deleteLater();
  const QPersistentModelIndex& index = watcher->index();
  if (index.isValid()) {
    emit dataChanged(index, index);
    emit EditingFinished(index);
  }
}

int Playlist::current_row() const {
  return current_item_index_.isValid() ? current_item_index_.row() : -1;
}

const QModelIndex Playlist::current_index() const {
  return current_item_index_;
}

int Playlist::last_played_row() const {
  return last_played_item_index_.isValid() ? last_played_item_index_.row() : -1;
}

void Playlist::ShuffleModeChanged(PlaylistSequence::ShuffleMode mode) {
  is_shuffled_ = (mode != PlaylistSequence::Shuffle_Off);
  ReshuffleIndices();
}

bool Playlist::FilterContainsVirtualIndex(int i) const {
  if (i<0 || i>=virtual_items_.count())
    return false;

  return proxy_->filterAcceptsRow(virtual_items_[i], QModelIndex());
}

int Playlist::NextVirtualIndex(int i) const {
  PlaylistSequence::RepeatMode repeat_mode = playlist_sequence_->repeat_mode();
  PlaylistSequence::ShuffleMode shuffle_mode = playlist_sequence_->shuffle_mode();
  bool album_only = repeat_mode == PlaylistSequence::Repeat_Album ||
                    shuffle_mode == PlaylistSequence::Shuffle_Album;

  // This one's easy - if we have to repeat the current track then just return i
  if (repeat_mode == PlaylistSequence::Repeat_Track) {
    if (!FilterContainsVirtualIndex(i))
      return virtual_items_.count(); // It's not in the filter any more
    return i;
  }

  // If we're not bothered about whether a song is on the same album then
  // return the next virtual index, whatever it is.
  if (!album_only) {
    ++i;

    // Advance i until we find any track that is in the filter
    while (i < virtual_items_.count() && !FilterContainsVirtualIndex(i))
      ++i;
    return i;
  }

  // We need to advance i until we get something else on the same album
  Song last_song = current_item_metadata();
  for (int j=i+1 ; j<virtual_items_.count(); ++j) {
    Song this_song = item_at(virtual_items_[j])->Metadata();
    if (((last_song.is_compilation() && this_song.is_compilation()) ||
         last_song.artist() == this_song.artist()) &&
        last_song.album() == this_song.album() &&
        FilterContainsVirtualIndex(j)) {
      return j; // Found one
    }
  }

  // Couldn't find one - return past the end of the list
  return virtual_items_.count();
}

int Playlist::PreviousVirtualIndex(int i) const {
  PlaylistSequence::RepeatMode repeat_mode = playlist_sequence_->repeat_mode();
  PlaylistSequence::ShuffleMode shuffle_mode = playlist_sequence_->shuffle_mode();
  bool album_only = repeat_mode == PlaylistSequence::Repeat_Album ||
                    shuffle_mode == PlaylistSequence::Shuffle_Album;

  // This one's easy - if we have to repeat the current track then just return i
  if (repeat_mode == PlaylistSequence::Repeat_Track) {
    if (!FilterContainsVirtualIndex(i))
      return -1;
    return i;
  }

  // If we're not bothered about whether a song is on the same album then
  // return the previous virtual index, whatever it is.
  if (!album_only) {
    --i;

    // Decrement i until we find any track that is in the filter
    while (i>=0 && !FilterContainsVirtualIndex(i))
      --i;
    return i;
  }

  // We need to decrement i until we get something else on the same album
  Song last_song = current_item_metadata();
  for (int j=i-1 ; j>=0; --j) {
    Song this_song = item_at(virtual_items_[j])->Metadata();
    if (((last_song.is_compilation() && this_song.is_compilation()) ||
         last_song.artist() == this_song.artist()) &&
        last_song.album() == this_song.album() &&
        FilterContainsVirtualIndex(j)) {
      return j; // Found one
    }
  }

  // Couldn't find one - return before the start of the list
  return -1;
}

int Playlist::next_row() const {
  // Did we want to stop after this track?
  if (stop_after_.isValid() && current_row() == stop_after_.row())
    return -1;

  // Any queued items take priority
  if (!queue_->is_empty()) {
    return queue_->PeekNext();
  }

  int next_virtual_index = NextVirtualIndex(current_virtual_index_);
  if (next_virtual_index >= virtual_items_.count()) {
    // We've gone off the end of the playlist.

    switch (playlist_sequence_->repeat_mode()) {
      case PlaylistSequence::Repeat_Off:
        return -1;
      case PlaylistSequence::Repeat_Track:
        next_virtual_index = current_virtual_index_;
        break;

      default:
        next_virtual_index = NextVirtualIndex(-1);
        break;
    }
  }

  // Still off the end?  Then just give up
  if (next_virtual_index < 0 || next_virtual_index >= virtual_items_.count())
    return -1;

  return virtual_items_[next_virtual_index];
}

int Playlist::previous_row() const {
  int prev_virtual_index = PreviousVirtualIndex(current_virtual_index_);
  if (prev_virtual_index < 0) {
    // We've gone off the beginning of the playlist.

    switch (playlist_sequence_->repeat_mode()) {
      case PlaylistSequence::Repeat_Off:
        return -1;
      case PlaylistSequence::Repeat_Track:
        prev_virtual_index = current_virtual_index_;
        break;

      default:
        prev_virtual_index = PreviousVirtualIndex(virtual_items_.count());
        break;
    }
  }

  // Still off the beginning?  Then just give up
  if (prev_virtual_index < 0)
    return -1;

  return virtual_items_[prev_virtual_index];
}

void Playlist::set_current_row(int i) {
  QModelIndex old_current = current_item_index_;
  ClearStreamMetadata();

  current_item_index_ = QPersistentModelIndex(index(i, 0, QModelIndex()));

  if (current_item_index_.row() == queue_->PeekNext()) {
    queue_->TakeNext();
  }

  if (current_item_index_ == old_current)
    return;

  if (current_item_index_.isValid()) {
    last_played_item_index_ = current_item_index_;
    current_item_ = items_[current_item_index_.row()];
    Save();
  } else {
    current_item_.reset();
  }

  if (old_current.isValid()) {
    if (dynamic_playlist_) {
      items_[old_current.row()]->SetDynamicHistory(true);
    }

    emit dataChanged(old_current, old_current.sibling(old_current.row(), ColumnCount-1));
  }

  if (current_item_index_.isValid()) {
    emit dataChanged(current_item_index_, current_item_index_.sibling(current_item_index_.row(), ColumnCount-1));
    emit CurrentSongChanged(current_item_metadata());
  }

  // Update the virtual index
  if (i == -1)
    current_virtual_index_ = -1;
  else if (is_shuffled_ && current_virtual_index_ == -1) {
    // This is the first thing we're playing so we want to make sure the array
    // is shuffled
    ReshuffleIndices();

    // Bring the one we've been asked to play to the start of the list
    virtual_items_.takeAt(virtual_items_.indexOf(i));
    virtual_items_.prepend(i);
    current_virtual_index_ = 0;
  } else if (is_shuffled_)
    current_virtual_index_ = virtual_items_.indexOf(i);
  else
    current_virtual_index_ = i;

  if (dynamic_playlist_ && current_item_index_.isValid() && old_current.isValid()) {
    using smart_playlists::Generator;

    // Add more dynamic playlist items
    const int count = current_item_index_.row() + Generator::kDynamicFuture - items_.count();
    if (count > 0) {
      GeneratorInserter* inserter = new GeneratorInserter(task_manager_, library_, this);
      connect(inserter, SIGNAL(Error(QString)), SIGNAL(LoadTracksError(QString)));
      connect(inserter, SIGNAL(PlayRequested(QModelIndex)), SIGNAL(PlayRequested(QModelIndex)));

      inserter->Load(this, -1, false, dynamic_playlist_, count);
    }

    // Remove the first item
    if (current_item_index_.row() > Generator::kDynamicHistory) {
      RemoveItemsWithoutUndo(0, 1);
    }
  }

  UpdateScrobblePoint();
}

Qt::ItemFlags Playlist::flags(const QModelIndex &index) const {
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if(column_is_editable((Column)index.column()))
    flags |= Qt::ItemIsEditable;

  if (index.isValid())
    return flags | Qt::ItemIsDragEnabled;

  return Qt::ItemIsDropEnabled;
}

QStringList Playlist::mimeTypes() const {
  return QStringList() << "text/uri-list" << kRowsMimetype
                       << LibraryModel::kSmartPlaylistsMimeType;
}

Qt::DropActions Playlist::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

bool Playlist::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int, const QModelIndex&) {
  if (action == Qt::IgnoreAction)
    return false;

  using smart_playlists::GeneratorMimeData;

  if (const SongMimeData* song_data = qobject_cast<const SongMimeData*>(data)) {
    // Dragged from a library
    // We want to check if these songs are from the actual local file backend,
    // if they are we treat them differently.
    if (song_data->backend && song_data->backend->songs_table() == Library::kSongsTable)
      InsertSongItems<LibraryPlaylistItem>(this, song_data->songs, row);
    else if (song_data->backend && song_data->backend->songs_table() == MagnatuneService::kSongsTable)
      InsertSongItems<MagnatunePlaylistItem>(this, song_data->songs, row);
    else if (song_data->backend && song_data->backend->songs_table() == JamendoService::kSongsTable)
      InsertSongItems<JamendoPlaylistItem>(this, song_data->songs, row);
    else
      InsertSongItems<SongPlaylistItem>(this, song_data->songs, row);
  } else if (const RadioMimeData* radio_data = qobject_cast<const RadioMimeData*>(data)) {
    // Dragged from the Radio pane
    InsertRadioStations(radio_data->model, radio_data->indexes,
                        row, data->hasFormat(kPlayNowMimetype));
  } else if (const GeneratorMimeData* generator_data = qobject_cast<const GeneratorMimeData*>(data)) {
    InsertSmartPlaylist(generator_data->generator_, row, data->hasFormat(kPlayNowMimetype));
  } else if (data->hasFormat(kRowsMimetype)) {
    // Dragged from the playlist
    // Rearranging it is tricky...

    // Get the list of rows that were moved
    QList<int> source_rows;
    Playlist* source_playlist = NULL;

    QDataStream stream(data->data(kRowsMimetype));
    stream >> source_playlist;
    stream >> source_rows;
    qStableSort(source_rows); // Make sure we take them in order

    if (source_playlist == this) {
      // Dragged from this playlist - rearrange the items
      undo_stack_->push(new PlaylistUndoCommands::MoveItems(this, source_rows, row));
    } else {
      // Drag from a different playlist
      PlaylistItemList items;
      foreach (int row, source_rows)
        items << source_playlist->item_at(row);
      undo_stack_->push(new PlaylistUndoCommands::InsertItems(this, items, row));

      // Remove the items from the source playlist if it was a move event
      if (action == Qt::MoveAction) {
        foreach (int row, source_rows) {
          source_playlist->undo_stack()->push(
              new PlaylistUndoCommands::RemoveItems(source_playlist, row, 1));
        }
      }
    }
  } else if (data->hasUrls()) {
    // URL list dragged from the file list or some other app
    InsertUrls(data->urls(), false, row);
  }

  return true;
}

void Playlist::InsertUrls(const QList<QUrl> &urls, bool play_now, int pos) {
  SongLoaderInserter* inserter = new SongLoaderInserter(task_manager_, library_, this);
  connect(inserter, SIGNAL(Error(QString)), SIGNAL(LoadTracksError(QString)));
  connect(inserter, SIGNAL(PlayRequested(QModelIndex)), SIGNAL(PlayRequested(QModelIndex)));

  inserter->Load(this, pos, play_now, urls);
}

void Playlist::InsertSmartPlaylist(GeneratorPtr generator, int pos, bool play_now) {
  GeneratorInserter* inserter = new GeneratorInserter(task_manager_, library_, this);
  connect(inserter, SIGNAL(Error(QString)), SIGNAL(LoadTracksError(QString)));
  connect(inserter, SIGNAL(PlayRequested(QModelIndex)), SIGNAL(PlayRequested(QModelIndex)));

  inserter->Load(this, pos, play_now, generator);

  if (generator->is_dynamic()) {
    TurnOnDynamicPlaylist(generator);
  }
}

void Playlist::TurnOnDynamicPlaylist(GeneratorPtr gen) {
  dynamic_playlist_ = gen;
  playlist_sequence_->SetUsingDynamicPlaylist(true);
  ShuffleModeChanged(PlaylistSequence::Shuffle_Off);
  emit DynamicModeChanged(true);
  Save();
}

void Playlist::MoveItemsWithoutUndo(const QList<int> &source_rows, int pos) {
  layoutAboutToBeChanged();
  PlaylistItemList moved_items;

  // Take the items out of the list first, keeping track of whether the
  // insertion point changes
  int offset = 0;
  foreach (int source_row, source_rows) {
    moved_items << items_.takeAt(source_row-offset);
    if (pos != -1 && pos >= source_row)
      pos --;
    offset++;
  }

  // Put the items back in
  const int start = pos == -1 ? items_.count() : pos;
  for (int i=start ; i<start+moved_items.count() ; ++i) {
    moved_items[i - start]->SetDynamicHistory(false);
    items_.insert(i, moved_items[i - start]);
  }

  // Update persistent indexes
  foreach (const QModelIndex& pidx, persistentIndexList()) {
    const int dest_offset = source_rows.indexOf(pidx.row());
    if (dest_offset != -1) {
      // This index was moved
      changePersistentIndex(pidx, index(start + dest_offset, pidx.column(), QModelIndex()));
    } else {
      int d = 0;
      foreach (int source_row, source_rows) {
        if (pidx.row() > source_row)
          d --;
      }
      if (pidx.row() + d >= start)
        d += source_rows.count();

      changePersistentIndex(pidx, index(pidx.row() + d, pidx.column(), QModelIndex()));
    }
  }
  current_virtual_index_ = virtual_items_.indexOf(current_row());

  layoutChanged();
  Save();
}

void Playlist::MoveItemsWithoutUndo(int start, const QList<int>& dest_rows) {
  layoutAboutToBeChanged();
  PlaylistItemList moved_items;

  if (start == -1)
    start = items_.count() - dest_rows.count();

  // Take the items out of the list first, keeping track of whether the
  // insertion point changes
  for (int i=start ; i<start + dest_rows.count() ; ++i)
    moved_items << items_.takeAt(start);

  // Put the items back in
  int offset = 0;
  foreach (int dest_row, dest_rows) {
    items_.insert(dest_row, moved_items[offset]);
    offset ++;
  }

  // Update persistent indexes
  foreach (const QModelIndex& pidx, persistentIndexList()) {
    if (pidx.row() >= start && pidx.row() < start + dest_rows.count()) {
      // This index was moved
      const int i = pidx.row() - start;
      changePersistentIndex(pidx, index(dest_rows[i], pidx.column(), QModelIndex()));
    } else {
      int d = 0;
      if (pidx.row() >= start + dest_rows.count())
        d -= dest_rows.count();

      foreach (int dest_row, dest_rows) {
        if (pidx.row() + d > dest_row)
          d ++;
      }

      changePersistentIndex(pidx, index(pidx.row() + d, pidx.column(), QModelIndex()));
    }
  }
  current_virtual_index_ = virtual_items_.indexOf(current_row());

  layoutChanged();
  Save();
}

QModelIndex Playlist::InsertItems(const PlaylistItemList& items, int pos, bool enqueue) {
  if (items.isEmpty())
    return QModelIndex();

  const int start = pos == -1 ? items_.count() : pos;
  undo_stack_->push(new PlaylistUndoCommands::InsertItems(this, items, pos, enqueue));

  return index(start, 0);
}

QModelIndex Playlist::InsertItemsWithoutUndo(const PlaylistItemList& items,
                                             int pos, bool enqueue) {
  if (items.isEmpty())
    return QModelIndex();

  const int start = pos == -1 ? items_.count() : pos;
  const int end = start + items.count() - 1;

  beginInsertRows(QModelIndex(), start, end);
  for (int i=start ; i<=end ; ++i) {
    PlaylistItemPtr item = items[i - start];
    items_.insert(i, item);
    virtual_items_ << virtual_items_.count();

    if (item->type() == "Library") {
      int id = item->Metadata().id();
      if (id != -1) {
        library_items_by_id_.insertMulti(id, item);
      }
    }

    if (item == current_item_) {
      // It's one we removed before that got re-added through an undo
      current_item_index_ = index(i, 0);
      last_played_item_index_ = current_item_index_;
    }
  }
  endInsertRows();

  if (enqueue) {
    QModelIndexList indexes;
    for (int i=start ; i<=end ; ++i) {
      indexes << index(i, 0);
    }
    queue_->ToggleTracks(indexes);
  }

  Save();
  ReshuffleIndices();

  return index(start, 0);
}

QModelIndex Playlist::InsertLibraryItems(const SongList& songs, int pos, bool enqueue) {
  return InsertSongItems<LibraryPlaylistItem>(this, songs, pos, enqueue);
}

QModelIndex Playlist::InsertSongs(const SongList& songs, int pos, bool enqueue) {
  return InsertSongItems<SongPlaylistItem>(this, songs, pos, enqueue);
}

QModelIndex Playlist::InsertSongsOrLibraryItems(const SongList& songs, int pos, bool enqueue) {
  PlaylistItemList items;
  foreach (const Song& song, songs) {
    if (song.id() == -1)
      items << PlaylistItemPtr(new SongPlaylistItem(song));
    else
      items << PlaylistItemPtr(new LibraryPlaylistItem(song));
  }
  return InsertItems(items, pos, enqueue);
}

QModelIndex Playlist::InsertRadioStations(
    const RadioModel* model, const QModelIndexList& items, int pos, bool play_now) {
  PlaylistItemList playlist_items;
  QList<QUrl> song_urls;

  foreach (const QModelIndex& item, items) {
    switch (item.data(RadioModel::Role_PlayBehaviour).toInt()) {
    case RadioModel::PlayBehaviour_SingleItem:
      playlist_items << shared_ptr<PlaylistItem>(new RadioPlaylistItem(
          model->ServiceForIndex(item),
          item.data(RadioModel::Role_Url).toUrl(),
          item.data(RadioModel::Role_Title).toString(),
          item.data(RadioModel::Role_Artist).toString()));
      break;

    case RadioModel::PlayBehaviour_UseSongLoader:
      song_urls << item.data(RadioModel::Role_Url).toUrl();
      break;
    }
  }

  if (!song_urls.isEmpty()) {
    InsertUrls(song_urls, play_now, pos);
  }

  return InsertItems(playlist_items, pos);
}

QMimeData* Playlist::mimeData(const QModelIndexList& indexes) const {
  QMimeData* data = new QMimeData;

  QList<QUrl> urls;
  QList<int> rows;
  foreach (const QModelIndex& index, indexes) {
    if (index.column() != 0)
      continue;

    urls << items_[index.row()]->Url();
    rows << index.row();
  }

  QBuffer buf;
  buf.open(QIODevice::WriteOnly);
  QDataStream stream(&buf);
  stream << this;
  stream << rows;
  buf.close();

  data->setUrls(urls);
  data->setData(kRowsMimetype, buf.data());

  return data;
}

bool Playlist::CompareItems(int column, Qt::SortOrder order,
                            shared_ptr<PlaylistItem> _a,
                            shared_ptr<PlaylistItem> _b) {
  shared_ptr<PlaylistItem> a = order == Qt::AscendingOrder ? _a : _b;
  shared_ptr<PlaylistItem> b = order == Qt::AscendingOrder ? _b : _a;

#define cmp(field)    return a->Metadata().field() < b->Metadata().field()
#define strcmp(field) return QString::localeAwareCompare( \
  a->Metadata().field().toLower(), b->Metadata().field().toLower()) < 0;

  switch (column) {
    case Column_Title:        strcmp(title);
    case Column_Artist:       strcmp(artist);
    case Column_Album:        strcmp(album);
    case Column_Length:       cmp(length);
    case Column_Track:        cmp(track);
    case Column_Disc:         cmp(disc);
    case Column_Year:         cmp(year);
    case Column_Genre:        strcmp(genre);
    case Column_AlbumArtist:  strcmp(albumartist);
    case Column_Composer:     strcmp(composer);

    case Column_Rating:       cmp(rating);
    case Column_PlayCount:    cmp(playcount);
    case Column_SkipCount:    cmp(skipcount);
    case Column_LastPlayed:   cmp(lastplayed);
    case Column_Score:        cmp(score);

    case Column_BPM:          cmp(bpm);
    case Column_Bitrate:      cmp(bitrate);
    case Column_Samplerate:   cmp(samplerate);
    case Column_Filename:     cmp(filename);
    case Column_BaseFilename: cmp(basefilename);
    case Column_Filesize:     cmp(filesize);
    case Column_Filetype:     cmp(filetype);
    case Column_DateModified: cmp(mtime);
    case Column_DateCreated:  cmp(ctime);

    case Column_Comment:      strcmp(comment);
  }

#undef cmp
#undef strcmp

  return false;
}

QString Playlist::column_name(Column column) {
  switch (column) {
    case Column_Title:        return tr("Title");
    case Column_Artist:       return tr("Artist");
    case Column_Album:        return tr("Album");
    case Column_Length:       return tr("Length");
    case Column_Track:        return tr("Track");
    case Column_Disc:         return tr("Disc");
    case Column_Year:         return tr("Year");
    case Column_Genre:        return tr("Genre");
    case Column_AlbumArtist:  return tr("Album artist");
    case Column_Composer:     return tr("Composer");

    case Column_Rating:       return tr("Rating");
    case Column_PlayCount:    return tr("Play count");
    case Column_SkipCount:    return tr("Skip count");
    case Column_LastPlayed:   return tr("Last played");
    case Column_Score:        return tr("Score");

    case Column_BPM:          return tr("BPM");
    case Column_Bitrate:      return tr("Bit rate");
    case Column_Samplerate:   return tr("Sample rate");
    case Column_Filename:     return tr("File name");
    case Column_BaseFilename: return tr("File name (without path)");
    case Column_Filesize:     return tr("File size");
    case Column_Filetype:     return tr("File type");
    case Column_DateModified: return tr("Date modified");
    case Column_DateCreated:  return tr("Date created");

    case Column_Comment:      return tr("Comment");
    default: return QString();
  }
  return "";
}

void Playlist::sort(int column, Qt::SortOrder order) {
  if (ignore_sorting_)
    return;

  layoutAboutToBeChanged();

  // This is a slow and nasty way to keep the persistent indices
  QMap<int, shared_ptr<PlaylistItem> > old_persistent_mappings;
  foreach (const QModelIndex& index, persistentIndexList()) {
    old_persistent_mappings[index.row()] = items_[index.row()];
  }

  qStableSort(items_.begin(), items_.end(),
              boost::bind(&Playlist::CompareItems, column, order, _1, _2));

  QMapIterator<int, shared_ptr<PlaylistItem> > it(old_persistent_mappings);
  while (it.hasNext()) {
    it.next();
    for (int col=0 ; col<ColumnCount ; ++col) {
      int new_row = items_.indexOf(it.value());
      changePersistentIndex(index(it.key(), col, QModelIndex()),
                            index(new_row, col, QModelIndex()));
    }
  }

  layoutChanged();

  // TODO
  undo_stack_->clear();

  emit PlaylistChanged();
  Save();
}

void Playlist::Playing() {
  SetCurrentIsPaused(false);
}

void Playlist::Paused() {
  SetCurrentIsPaused(true);
}

void Playlist::Stopped() {
  SetCurrentIsPaused(false);
}

void Playlist::SetCurrentIsPaused(bool paused) {
  if (paused == current_is_paused_)
    return;

  current_is_paused_ = paused;

  if (current_item_index_.isValid())
    dataChanged(index(current_item_index_.row(), 0),
                index(current_item_index_.row(), ColumnCount-1));
}

void Playlist::Save() const {
  if (!backend_ || is_loading_)
    return;

  backend_->SavePlaylistAsync(id_, items_, last_played_row(), dynamic_playlist_);
}

namespace {
typedef QFutureWatcher<shared_ptr<PlaylistItem> > PlaylistItemFutureWatcher;
}

void Playlist::Restore() {
  if (!backend_)
    return;

  items_.clear();
  virtual_items_.clear();
  library_items_by_id_.clear();

  PlaylistBackend::PlaylistItemFuture future = backend_->GetPlaylistItems(id_);
  PlaylistItemFutureWatcher* watcher = new PlaylistItemFutureWatcher(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(ItemsLoaded()));
}

void Playlist::ItemsLoaded() {
  PlaylistItemFutureWatcher* watcher = static_cast<PlaylistItemFutureWatcher*>(sender());
  watcher->deleteLater();

  PlaylistItemList items = watcher->future().results();
  is_loading_ = true;
  InsertItems(items, 0);
  is_loading_ = false;

  PlaylistBackend::Playlist p = backend_->GetPlaylist(id_);

  last_played_item_index_ =
      p.last_played == -1 ? QModelIndex() : index(p.last_played);

  if (!p.dynamic_type.isEmpty()) {
    GeneratorPtr gen = Generator::Create(p.dynamic_type);
    if (gen) {
      // Hack: can't think of a better way to get the right backend
      LibraryBackend* backend = NULL;
      if (p.dynamic_backend == library_->songs_table())
        backend = library_;
      else if (p.dynamic_backend == MagnatuneService::kSongsTable)
        backend = RadioModel::Service<MagnatuneService>()->library_backend();
      else if (p.dynamic_backend == JamendoService::kSongsTable)
        backend = RadioModel::Service<JamendoService>()->library_backend();

      if (backend) {
        gen->set_library(backend);
        gen->Load(p.dynamic_data);
        TurnOnDynamicPlaylist(gen);
      }
    }
  }
}

bool Playlist::removeRows(int row, int count, const QModelIndex& parent) {
  if (row < 0 || row >= items_.size() || row + count > items_.size()) {
    return false;
  }

  undo_stack_->push(new PlaylistUndoCommands::RemoveItems(this, row, count));
  return true;
}

PlaylistItemList Playlist::RemoveItemsWithoutUndo(int row, int count) {
  if (row < 0 || row >= items_.size() || row + count > items_.size()) {
    return PlaylistItemList();
  }
  beginRemoveRows(QModelIndex(), row, row+count-1);

  // Remove items
  PlaylistItemList ret;
  for (int i=0 ; i<count ; ++i) {
    PlaylistItemPtr item(items_.takeAt(row));
    ret << item;

    if (item->type() == "Library") {
      int id = item->Metadata().id();
      if (id != -1) {
        library_items_by_id_.remove(id, item);
      }
    }
  }

  endRemoveRows();

  QList<int>::iterator it = virtual_items_.begin();
  int i = 0;
  while (it != virtual_items_.end()) {
    if (*it >= items_.count())
      it = virtual_items_.erase(it);
    else
      ++it;
    ++i;
  }

  // Reset current_virtual_index_
  if (current_row() == -1)
    current_virtual_index_ = -1;
  else
    current_virtual_index_ = virtual_items_.indexOf(current_row());

  Save();
  return ret;
}

void Playlist::StopAfter(int row) {
  QModelIndex old_stop_after = stop_after_;

  if ((stop_after_.isValid() && stop_after_.row() == row) || row == -1)
    stop_after_ = QModelIndex();
  else
    stop_after_ = index(row, 0);

  if (old_stop_after.isValid())
    emit dataChanged(old_stop_after, old_stop_after.sibling(old_stop_after.row(), ColumnCount-1));
  if (stop_after_.isValid())
    emit dataChanged(stop_after_, stop_after_.sibling(stop_after_.row(), ColumnCount-1));
}

void Playlist::SetStreamMetadata(const QUrl& url, const Song& song) {
  if (!current_item_)
    return;

  if (current_item_->Url() != url)
    return;

  // Don't update the metadata if it's only a minor change from before
  if (current_item_->Metadata().artist() == song.artist() &&
      current_item_->Metadata().title() == song.title())
    return;

  current_item_->SetTemporaryMetadata(song);
  UpdateScrobblePoint();

  emit dataChanged(index(current_item_index_.row(), 0), index(current_item_index_.row(), ColumnCount-1));
  emit CurrentSongChanged(song);
}

void Playlist::ClearStreamMetadata() {
  if (!current_item_)
    return;

  current_item_->ClearTemporaryMetadata();
  UpdateScrobblePoint();

  emit dataChanged(index(current_item_index_.row(), 0), index(current_item_index_.row(), ColumnCount-1));
}

bool Playlist::stop_after_current() const {
  return stop_after_.isValid() && current_item_index_.isValid() &&
         stop_after_.row() == current_item_index_.row();
}

PlaylistItem::Options Playlist::current_item_options() const {
  if (!current_item_)
    return PlaylistItem::Default;

  return current_item_->options();
}

Song Playlist::current_item_metadata() const {
  if (!current_item_)
    return Song();

  return current_item_->Metadata();
}

void Playlist::UpdateScrobblePoint() {
  const int length = current_item_metadata().length();

  scrobble_point_ = length == 0 ? 240 : qBound(31, length/2, 240);
  has_scrobbled_ = false;
}

void Playlist::Clear() {
  undo_stack_->push(new PlaylistUndoCommands::RemoveItems(this, 0, items_.count()));
  TurnOffDynamicPlaylist();

  Save();
}

void Playlist::TurnOffDynamicPlaylist() {
  dynamic_playlist_.reset();

  if (playlist_sequence_) {
    playlist_sequence_->SetUsingDynamicPlaylist(false);
    ShuffleModeChanged(playlist_sequence_->shuffle_mode());
  }
  emit DynamicModeChanged(false);
  Save();
}

void Playlist::RepopulateDynamicPlaylist() {
  if (!dynamic_playlist_)
    return;

  RemoveItemsNotInQueue();
  InsertSmartPlaylist(dynamic_playlist_);
}

void Playlist::RemoveItemsNotInQueue() {
  if (queue_->is_empty()) {
    RemoveItemsWithoutUndo(0, items_.count());
    return;
  }

  int start = 0;
  forever {
    // Find a place to start - first row that isn't in the queue
    forever {
      if (start >= rowCount())
        return;
      if (!queue_->ContainsSourceRow(start))
        break;
      start ++;
    }

    // Figure out how many rows to remove - keep going until we find a row
    // that is in the queue
    int count = 1;
    forever {
      if (start + count >= rowCount())
        break;
      if (queue_->ContainsSourceRow(start + count))
        break;
      count ++;
    }

    RemoveItemsWithoutUndo(start, count);
    start ++;
  }
}

void Playlist::ReloadItems(const QList<int>& rows) {
  foreach (int row, rows) {
    item_at(row)->Reload();
    emit dataChanged(index(row, 0), index(row, ColumnCount-1));
  }
}

void Playlist::RateSong(const QModelIndex& index, double rating) {
  int row = index.row();

  if(has_item_at(row)) {
    PlaylistItemPtr item = item_at(row);
    if (item && item->IsLocalLibraryItem()) {
      library_->UpdateSongRatingAsync(item->Metadata().id(), rating);
    }
  }
}

void Playlist::Shuffle() {
  layoutAboutToBeChanged();

  const int count = items_.count();
  for (int i=0 ; i < count; ++i) {
    int new_pos = i + (rand() % (count - i));

    std::swap(items_[i], items_[new_pos]);

    foreach (const QModelIndex& pidx, persistentIndexList()) {
      if (pidx.row() == i)
        changePersistentIndex(pidx, index(new_pos, pidx.column(), QModelIndex()));
      else if (pidx.row() == new_pos)
        changePersistentIndex(pidx, index(i, pidx.column(), QModelIndex()));
    }
  }
  current_virtual_index_ = virtual_items_.indexOf(current_row());

  layoutChanged();

  // TODO
  undo_stack_->clear();

  emit PlaylistChanged();
  Save();
}

void Playlist::ReshuffleIndices() {
  if (!is_shuffled_) {
    std::sort(virtual_items_.begin(), virtual_items_.end());
    if (current_row() != -1)
      current_virtual_index_ = virtual_items_.indexOf(current_row());
  } else {
    QList<int>::iterator begin = virtual_items_.begin();
    if (current_virtual_index_ != -1)
      std::advance(begin, current_virtual_index_ + 1);

    std::random_shuffle(begin, virtual_items_.end());
  }
}

void Playlist::set_sequence(PlaylistSequence* v) {
  playlist_sequence_ = v;
  connect(v, SIGNAL(ShuffleModeChanged(PlaylistSequence::ShuffleMode)),
          SLOT(ShuffleModeChanged(PlaylistSequence::ShuffleMode)));

  ShuffleModeChanged(v->shuffle_mode());
}

QSortFilterProxyModel* Playlist::proxy() const {
  return proxy_;
}

SongList Playlist::GetAllSongs() const {
  SongList ret;
  foreach (PlaylistItemPtr item, items_) {
    ret << item->Metadata();
  }
  return ret;
}

quint64 Playlist::GetTotalLength() const {
  quint64 ret = 0;
  foreach (PlaylistItemPtr item, items_) {
    int length = item->Metadata().length();
    if (length > 0)
      ret += length;
  }
  return ret;
}

PlaylistItemList Playlist::library_items_by_id(int id) const {
  return library_items_by_id_.values(id);
}

void Playlist::TracksAboutToBeDequeued(const QModelIndex&, int begin, int end) {
  for (int i=begin ; i<=end ; ++i) {
    temp_dequeue_change_indexes_ << queue_->mapToSource(queue_->index(i, Column_Title));
  }
}

void Playlist::TracksDequeued() {
  foreach (const QModelIndex& index, temp_dequeue_change_indexes_) {
    emit dataChanged(index, index);
  }
  temp_dequeue_change_indexes_.clear();
}

void Playlist::TracksEnqueued(const QModelIndex&, int begin, int end) {
  const QModelIndex& b = queue_->mapToSource(queue_->index(begin, Column_Title));
  const QModelIndex& e = queue_->mapToSource(queue_->index(end, Column_Title));
  emit dataChanged(b, e);
}

void Playlist::QueueLayoutChanged() {
  for (int i=0 ; i<queue_->rowCount() ; ++i) {
    const QModelIndex& index = queue_->mapToSource(queue_->index(i, Column_Title));
    emit dataChanged(index, index);
  }
}

QDataStream& operator <<(QDataStream& s, const Playlist* p) {
  s.writeRawData(reinterpret_cast<char*>(&p), sizeof(p));
  return s;
}

QDataStream& operator >>(QDataStream& s, Playlist*& p) {
  s.readRawData(reinterpret_cast<char*>(&p), sizeof(p));
  return s;
}

void Playlist::ItemChanged(PlaylistItemPtr item) {
  for (int row=0 ; row<items_.count() ; ++row) {
    if (items_[row] == item) {
      emit dataChanged(index(row, 0), index(row, ColumnCount-1));
      return;
    }
  }
}

void Playlist::set_column_alignments(const ColumnAlignmentMap& column_alignments) {
  column_alignments_ = column_alignments;
}

void Playlist::set_column_align_left(int column) {
  column_alignments_[column] = (Qt::AlignLeft | Qt::AlignVCenter);
}

void Playlist::set_column_align_center(int column) {
  column_alignments_[column] = Qt::AlignCenter;
}

void Playlist::set_column_align_right(int column) {
  column_alignments_[column] = (Qt::AlignRight | Qt::AlignVCenter);
}
