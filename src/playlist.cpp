#include "playlist.h"
#include "songmimedata.h"
#include "songplaylistitem.h"
#include "radiomimedata.h"
#include "radioplaylistitem.h"
#include "radiomodel.h"
#include "savedradio.h"

#include <QtDebug>
#include <QMimeData>
#include <QBuffer>
#include <QSettings>
#include <QFileInfo>
#include <QDirIterator>

#include <boost/bind.hpp>
#include <algorithm>

#include <lastfm/ScrobblePoint>

const char* Playlist::kRowsMimetype = "application/x-clementine-playlist-rows";
const char* Playlist::kSettingsGroup = "Playlist";

Playlist::Playlist(QObject *parent) :
    QAbstractListModel(parent),
    current_is_paused_(false),
    current_virtual_index_(-1),
    is_shuffled_(false),
    scrobble_point_(-1),
    has_scrobbled_(false),
    playlist_sequence_(NULL),
    ignore_sorting_(false)
{
}

Playlist::~Playlist() {
  qDeleteAll(items_);
}

QVariant Playlist::headerData(int section, Qt::Orientation, int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  switch (section) {
    case Column_Title:    return tr("Title");
    case Column_Artist:   return tr("Artist");
    case Column_Album:    return tr("Album");
    case Column_Length:   return tr("Length");
    case Column_Track:    return tr("Track");
    case Column_Disc:     return tr("Disc");
    case Column_Year:     return tr("Year");
    case Column_Genre:    return tr("Genre");
    case Column_AlbumArtist: return tr("Album artist");
    case Column_Composer: return tr("Composer");

    case Column_BPM:      return tr("BPM");
    case Column_Bitrate:  return tr("Bit rate");
    case Column_Samplerate: return tr("Sample rate");
    case Column_Filename: return tr("File name");
    case Column_Filesize: return tr("File size");
    case Column_Filetype: return tr("File type");
    case Column_DateModified: return tr("Date modified");
    case Column_DateCreated: return tr("Date created");
  }

  return QVariant();
}

QVariant Playlist::data(const QModelIndex& index, int role) const {
  switch (role) {
    case Role_IsCurrent:
      return current_item_.isValid() && index.row() == current_item_.row();

    case Role_IsPaused:
      return current_is_paused_;

    case Role_StopAfter:
      return stop_after_.isValid() && stop_after_.row() == index.row();

    case Qt::DisplayRole: {
      PlaylistItem* item = items_[index.row()];
      Song song = item->Metadata();

      switch (index.column()) {
        case Column_Title:    return song.title();
        case Column_Artist:   return song.artist();
        case Column_Album:    return song.album();
        case Column_Length:   return song.length();
        case Column_Track:    return song.track();
        case Column_Disc:     return song.disc();
        case Column_Year:     return song.year();
        case Column_Genre:    return song.genre();
        case Column_AlbumArtist: return song.albumartist();
        case Column_Composer: return song.composer();

        case Column_BPM:      return song.bpm();
        case Column_Bitrate:  return song.bitrate();
        case Column_Samplerate: return song.samplerate();
        case Column_Filename: return song.filename();
        case Column_Filesize: return song.filesize();
        case Column_Filetype: return song.filetype();
        case Column_DateModified: return song.mtime();
        case Column_DateCreated: return song.ctime();
      }
    }

    default:
      return QVariant();
  }
}

int Playlist::current_index() const {
  return current_item_.isValid() ? current_item_.row() : -1;
}

void Playlist::ShuffleModeChanged(PlaylistSequence::ShuffleMode mode) {
  is_shuffled_ = (mode != PlaylistSequence::Shuffle_Off);
  ReshuffleIndices();
}

int Playlist::NextVirtualIndex(int i) const {
  PlaylistSequence::RepeatMode repeat_mode = playlist_sequence_->repeat_mode();
  PlaylistSequence::ShuffleMode shuffle_mode = playlist_sequence_->shuffle_mode();
  bool album_only = repeat_mode == PlaylistSequence::Repeat_Album ||
                    shuffle_mode == PlaylistSequence::Shuffle_Album;

  // This one's easy - if we have to repeat the current track then just return i
  if (repeat_mode == PlaylistSequence::Repeat_Track)
    return i;

  // If we're not bothered about whether a song is on the same album then
  // return the next virtual index, whatever it is.
  if (!album_only)
    return i+1;

  // We need to advance i until we get something else on the same album
  Song last_song = current_item_metadata();
  for (int j=i+1 ; j<virtual_items_.count(); ++j) {
    Song this_song = item_at(virtual_items_[j])->Metadata();
    if ((last_song.is_compilation() && this_song.is_compilation() ||
         last_song.artist() == this_song.artist()) &&
        last_song.album() == this_song.album()) {
      return j; // Found one
    }
  }

  // Couldn't find one - return past the end of the list
  return virtual_items_.count();
}

int Playlist::next_index() const {
  // Did we want to stop after this track?
  if (stop_after_.isValid() && current_index() == stop_after_.row())
    return -1;

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
  if (next_virtual_index >= virtual_items_.count())
    return -1;

  return virtual_items_[next_virtual_index];
}

int Playlist::previous_index() const {
  int i = current_index() - 1;
  if (i < 0)
    return -1;
  return i;
}

void Playlist::set_current_index(int i) {
  QModelIndex old_current = current_item_;
  ClearStreamMetadata();

  current_item_ = QPersistentModelIndex(index(i, 0, QModelIndex()));

  if (old_current.isValid())
    emit dataChanged(old_current, old_current.sibling(old_current.row(), ColumnCount));

  if (current_item_.isValid() && current_item_ != old_current) {
    emit dataChanged(current_item_, current_item_.sibling(current_item_.row(), ColumnCount));
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

  UpdateScrobblePoint();
}

Qt::ItemFlags Playlist::flags(const QModelIndex &index) const {
  if (index.isValid())
    return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QStringList Playlist::mimeTypes() const {
  return QStringList() << "text/uri-list" << kRowsMimetype;
}

Qt::DropActions Playlist::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction;
}

bool Playlist::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int, const QModelIndex&) {
  if (action == Qt::IgnoreAction)
    return false;

  if (const SongMimeData* song_data = qobject_cast<const SongMimeData*>(data)) {
    // Dragged from the library
    InsertSongs(song_data->songs, row);
  } else if (const RadioMimeData* radio_data = qobject_cast<const RadioMimeData*>(data)) {
    // Dragged from the Radio pane
    InsertRadioStations(radio_data->items, row);
  } else if (data->hasFormat(kRowsMimetype)) {
    // Dragged from the playlist
    // Rearranging it is tricky...

    // Get the list of rows that were moved
    QList<int> source_rows;
    QDataStream stream(data->data(kRowsMimetype));
    stream >> source_rows;
    qStableSort(source_rows); // Make sure we take them in order

    layoutAboutToBeChanged();
    QList<PlaylistItem*> moved_items;

    // Take the items out of the list first, keeping track of whether the
    // insertion point changes
    int offset = 0;
    foreach (int source_row, source_rows) {
      moved_items << items_.takeAt(source_row-offset);
      if (row != -1 && row >= source_row)
        row --;
      offset++;
    }

    // Put the items back in
    const int start = row == -1 ? items_.count() : row;
    for (int i=start ; i<start+moved_items.count() ; ++i) {
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
    current_virtual_index_ = virtual_items_.indexOf(current_index());

    layoutChanged();
    Save();

  } else if (data->hasUrls()) {
    // URL list dragged from the file list or some other app
    InsertPaths(data->urls(), row);
  }

  return true;
}

QModelIndex Playlist::InsertPaths(QList<QUrl> urls, int after) {
  SongList songs;
  for (int i=0 ; i<urls.count() ; ++i) {
    QUrl url(urls[i]);
    if (url.scheme() != "file")
      continue;

    QString filename(url.toLocalFile());
    QFileInfo info(filename);

    if (!info.exists())
      continue;

    if (info.isDir()) {
      // Add all the songs in the directory
      QDirIterator it(filename,
          QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
          QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

      QList<QUrl> new_urls;
      while (it.hasNext()) {
        QString path(it.next());
        new_urls << QUrl::fromLocalFile(path);
      }
      qSort(new_urls);
      urls << new_urls;
    } else {
      Song song;
      song.InitFromFile(filename, -1);
      if (!song.is_valid())
        continue;

      songs << song;
    }
  }

  return InsertSongs(songs, after);
}

QModelIndex Playlist::InsertItems(const QList<PlaylistItem*>& items, int after) {
  if (items.isEmpty())
    return QModelIndex();

  const int start = after == -1 ? items_.count() : after;
  const int end = start + items.count() - 1;

  beginInsertRows(QModelIndex(), start, end);
  for (int i=start ; i<=end ; ++i) {
    items_.insert(i, items[i - start]);
    virtual_items_ << virtual_items_.count();
  }
  endInsertRows();

  Save();
  ReshuffleIndices();

  return index(start, 0);
}

QModelIndex Playlist::InsertSongs(const SongList& songs, int after) {
  QList<PlaylistItem*> items;
  foreach (const Song& song, songs) {
    items << new SongPlaylistItem(song);
  }
  return InsertItems(items, after);
}

QModelIndex Playlist::InsertRadioStations(const QList<RadioItem*>& items, int after) {
  QList<PlaylistItem*> playlist_items;
  foreach (RadioItem* item, items) {
    if (!item->playable)
      continue;

    playlist_items << new RadioPlaylistItem(item->service, item->Url(), item->Title(), item->Artist());
  }
  return InsertItems(playlist_items, after);
}

QModelIndex Playlist::InsertStreamUrls(const QList<QUrl>& urls, int after) {
  QList<PlaylistItem*> playlist_items;
  foreach (const QUrl& url, urls) {
    playlist_items << new RadioPlaylistItem(
        RadioModel::ServiceByName(SavedRadio::kServiceName), url.toString(), url.toString(), QString());
  }
  return InsertItems(playlist_items, after);
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
  stream << rows;
  buf.close();

  data->setUrls(urls);
  data->setData(kRowsMimetype, buf.data());

  return data;
}

bool Playlist::CompareItems(int column, Qt::SortOrder order,
                            const PlaylistItem* _a, const PlaylistItem* _b) {
  const PlaylistItem* a = order == Qt::AscendingOrder ? _a : _b;
  const PlaylistItem* b = order == Qt::AscendingOrder ? _b : _a;

  switch (column) {
    case Column_Title:  return a->Metadata().title() < b->Metadata().title();
    case Column_Artist: return a->Metadata().artist() < b->Metadata().artist();
    case Column_Album:  return a->Metadata().album() < b->Metadata().album();
    case Column_Length: return a->Metadata().length() < b->Metadata().length();
    case Column_Track:  return a->Metadata().track() < b->Metadata().track();
  }
  return false;
}

void Playlist::sort(int column, Qt::SortOrder order) {
  if (ignore_sorting_)
    return;

  layoutAboutToBeChanged();

  // This is a slow and nasty way to keep the persistent indices
  QMap<int, PlaylistItem*> old_persistent_mappings;
  foreach (const QModelIndex& index, persistentIndexList()) {
    old_persistent_mappings[index.row()] = items_[index.row()];
  }

  qStableSort(items_.begin(), items_.end(),
              boost::bind(&Playlist::CompareItems, column, order, _1, _2));

  QMapIterator<int, PlaylistItem*> it(old_persistent_mappings);
  while (it.hasNext()) {
    it.next();
    for (int col=0 ; col<ColumnCount ; ++col) {
      int new_row = items_.indexOf(it.value());
      changePersistentIndex(index(it.key(), col, QModelIndex()),
                            index(new_row, col, QModelIndex()));
    }
  }

  layoutChanged();

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

  if (current_item_.isValid())
    dataChanged(index(current_item_.row(), 0),
                index(current_item_.row(), ColumnCount));
}

void Playlist::Save() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.beginWriteArray("items", items_.count());
  for (int i=0 ; i<items_.count() ; ++i) {
    s.setArrayIndex(i);
    s.setValue("type", items_.at(i)->type_string());
    items_.at(i)->Save(s);
  }
  s.endArray();
}

void Playlist::Restore() {
  qDeleteAll(items_);
  items_.clear();
  virtual_items_.clear();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  int count = s.beginReadArray("items");
  for (int i=0 ; i<count ; ++i) {
    s.setArrayIndex(i);
    QString type(s.value("type").toString());

    PlaylistItem* item = PlaylistItem::NewFromType(type);
    if (!item)
      continue;

    item->Restore(s);
    items_ << item;
    virtual_items_ << virtual_items_.count();
  }
  s.endArray();

  reset();
}

bool Playlist::removeRows(int row, int count, const QModelIndex& parent) {
  beginRemoveRows(parent, row, row+count-1);

  // Remove items
  for (int i=0 ; i<count ; ++i)
    delete items_.takeAt(row);

  endRemoveRows();

  QList<int>::iterator it = virtual_items_.begin();
  int i = 0;
  while (it != virtual_items_.end()) {
    if (*it >= items_.count()) {
      if (i >= current_virtual_index_)
        current_virtual_index_ --;

      it = virtual_items_.erase(it);
    } else {
      ++it;
    }
    ++i;
  }

  Save();
  return true;
}

void Playlist::StopAfter(int row) {
  QModelIndex old_stop_after = stop_after_;

  if ((stop_after_.isValid() && stop_after_.row() == row) || row == -1)
    stop_after_ = QModelIndex();
  else
    stop_after_ = index(row, 0);

  if (old_stop_after.isValid())
    emit dataChanged(old_stop_after, old_stop_after.sibling(old_stop_after.row(), ColumnCount));
  if (stop_after_.isValid())
    emit dataChanged(stop_after_, stop_after_.sibling(stop_after_.row(), ColumnCount));
}

void Playlist::SetStreamMetadata(const QUrl& url, const Song& song) {
  if (!current_item_.isValid())
    return;

  PlaylistItem* item = items_[current_item_.row()];
  if (item->Url() != url)
    return;

  // Don't update the metadata if it's only a minor change from before
  if (item->Metadata().artist() == song.artist() &&
      item->Metadata().title() == song.title())
    return;

  item->SetTemporaryMetadata(song);
  UpdateScrobblePoint();

  emit dataChanged(index(current_item_.row(), 0), index(current_item_.row(), ColumnCount));
  emit CurrentSongChanged(song);
}

void Playlist::ClearStreamMetadata() {
  if (!current_item_.isValid())
    return;

  PlaylistItem* item = items_[current_item_.row()];
  item->ClearTemporaryMetadata();
  UpdateScrobblePoint();

  emit dataChanged(index(current_item_.row(), 0), index(current_item_.row(), ColumnCount));
}

bool Playlist::stop_after_current() const {
  return stop_after_.isValid() && current_item_.isValid() &&
         stop_after_.row() == current_item_.row();
}

PlaylistItem* Playlist::current_item() const {
  int i = current_index();
  if (i == -1)
    return NULL;

  return item_at(i);
}

PlaylistItem::Options Playlist::current_item_options() const {
  PlaylistItem* item = current_item();
  if (!item)
    return PlaylistItem::Default;

  return item->options();
}

Song Playlist::current_item_metadata() const {
  PlaylistItem* item = current_item();
  if (!item)
    return Song();

  return item->Metadata();
}

void Playlist::UpdateScrobblePoint() {
  int length = qMin(current_item_metadata().length(), 240);

  ScrobblePoint point(length / 2);
  scrobble_point_ = point;
  has_scrobbled_ = false;
}

void Playlist::Clear() {
  qDeleteAll(items_);
  items_.clear();
  virtual_items_.clear();
  reset();

  Save();
}

void Playlist::ReloadItems(const QList<int>& rows) {
  foreach (int row, rows) {
    item_at(row)->Reload();
    emit dataChanged(index(row, 0), index(row, ColumnCount-1));
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

  layoutChanged();

  Save();
}

void Playlist::ReshuffleIndices() {
  if (!is_shuffled_) {
    std::sort(virtual_items_.begin(), virtual_items_.end());
    if (current_index() != -1)
      current_virtual_index_ = virtual_items_.indexOf(current_index());
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
