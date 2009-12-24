#include "playlist.h"
#include "songmimedata.h"
#include "songplaylistitem.h"

#include <QtDebug>
#include <QMimeData>
#include <QBuffer>
#include <QSettings>
#include <QFileInfo>
#include <QDirIterator>

#include <boost/bind.hpp>

const char* Playlist::kRowsMimetype = "application/x-tangerine-playlist-rows";
const char* Playlist::kSettingsGroup = "Playlist";

Playlist::Playlist(QObject *parent) :
    QAbstractListModel(parent),
    current_is_paused_(false),
    ignore_next_sort_(true)
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

      switch (index.column()) {
        case Column_Title:    return item->Title();
        case Column_Artist:   return item->Artist();
        case Column_Album:    return item->Album();
        case Column_Length:   return item->Length();
        case Column_Track:    return item->Track();
      }
    }

    default:
      return QVariant();
  }
}

int Playlist::current_item() const {
  return current_item_.isValid() ? current_item_.row() : -1;
}

int Playlist::next_item() const {
  int i = current_item() + 1;
  if (i >= items_.count())
    return -1;
  if (stop_after_.isValid() && current_item() == stop_after_.row())
    return -1;

  return i;
}

int Playlist::previous_item() const {
  int i = current_item() - 1;
  if (i < 0)
    return -1;
  return i;
}

void Playlist::set_current_item(int i) {
  QModelIndex old_current = current_item_;
  current_item_ = QPersistentModelIndex(index(i, 0, QModelIndex()));

  if (old_current.isValid())
    emit dataChanged(old_current, old_current.sibling(old_current.row(), ColumnCount));
  if (current_item_.isValid())
    emit dataChanged(current_item_, current_item_.sibling(current_item_.row(), ColumnCount));
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

  QList<PlaylistItem*> added_items;
  if (const SongMimeData* song_data = qobject_cast<const SongMimeData*>(data)) {
    // Dragged from the library
    InsertSongs(song_data->songs, row);
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

      // Update persistent indexes
      for (int column=0 ; column<ColumnCount ; ++column)
        changePersistentIndex(index(source_rows[i-start], column, QModelIndex()),
                              index(i, column, QModelIndex()));
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
    case Column_Title:  return a->Title() < b->Title();
    case Column_Artist: return a->Artist() < b->Artist();
    case Column_Album:  return a->Album() < b->Album();
    case Column_Length: return a->Length() < b->Length();
    case Column_Track:  return a->Track() < b->Track();
  }
  return false;
}

void Playlist::sort(int column, Qt::SortOrder order) {
  if (ignore_next_sort_) {
    ignore_next_sort_ = false;
    return;
  }

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
    items_.removeAt(row);

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
