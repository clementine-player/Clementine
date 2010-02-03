#include "playlist.h"
#include "songmimedata.h"
#include "songplaylistitem.h"
#include "radiomimedata.h"
#include "radioplaylistitem.h"

#include <QtDebug>
#include <QMimeData>
#include <QBuffer>
#include <QSettings>
#include <QFileInfo>
#include <QDirIterator>

#include <boost/bind.hpp>

#include <lastfm/ScrobblePoint>

const char* Playlist::kRowsMimetype = "application/x-clementine-playlist-rows";
const char* Playlist::kSettingsGroup = "Playlist";

Playlist::Playlist(QObject *parent) :
    QAbstractListModel(parent),
    current_is_paused_(false),
    scrobble_point_(-1),
    has_scrobbled_(false),
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
    case Column_Title:    return "Title";
    case Column_Artist:   return "Artist";
    case Column_Album:    return "Album";
    case Column_Length:   return "Length";
    case Column_Track:    return "Track";
    case Column_Disc:     return "Disc";
    case Column_Year:     return "Year";
    case Column_Genre:    return "Genre";

    case Column_BPM:      return "BPM";
    case Column_Bitrate:  return "Bit rate";
    case Column_Samplerate: return "Sample rate";
    case Column_Filename: return "File name";
    case Column_Filesize: return "File size";
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

        case Column_BPM:      return song.bpm();
        case Column_Bitrate:  return song.bitrate();
        case Column_Samplerate: return song.samplerate();
        case Column_Filename: return song.filename();
        case Column_Filesize: return song.filesize();
      }
    }

    default:
      return QVariant();
  }
}

int Playlist::current_index() const {
  return current_item_.isValid() ? current_item_.row() : -1;
}

int Playlist::next_index() const {
  int i = current_index() + 1;
  if (i >= items_.count())
    return -1;
  if (stop_after_.isValid() && current_index() == stop_after_.row())
    return -1;

  return i;
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

  if (current_item_.isValid()) {
    emit dataChanged(current_item_, current_item_.sibling(current_item_.row(), ColumnCount));
    emit CurrentSongChanged(current_item_metadata());
  }

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

    layoutChanged();

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
  }

  endInsertRows();

  Save();

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
  int length = current_item_metadata().length();

  ScrobblePoint point(length / 2);
  scrobble_point_ = point;
  has_scrobbled_ = false;
}

void Playlist::Clear() {
  qDeleteAll(items_);
  items_.clear();
  reset();
}

void Playlist::ReloadItems(const QList<int>& rows) {
  foreach (int row, rows) {
    item_at(row)->Reload();
    emit dataChanged(index(row, 0), index(row, ColumnCount-1));
  }
}
