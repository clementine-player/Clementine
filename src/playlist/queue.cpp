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

#include "queue.h"

#include <algorithm>

#include <QBuffer>
#include <QMimeData>
#include <QtDebug>

#include "core/utilities.h"

const char* Queue::kRowsMimetype = "application/x-clementine-queue-rows";

Queue::Queue(Playlist* parent)
    : QAbstractProxyModel(parent), playlist_(parent), total_length_ns_(0) {
  connect(this, SIGNAL(ItemCountChanged(int)), SLOT(UpdateTotalLength()));
  connect(this, SIGNAL(TotalLengthChanged(quint64)), SLOT(UpdateSummaryText()));

  UpdateSummaryText();
}

QModelIndex Queue::mapFromSource(const QModelIndex& source_index) const {
  if (!source_index.isValid()) return QModelIndex();

  const int source_row = source_index.row();
  for (int i = 0; i < source_indexes_.count(); ++i) {
    if (source_indexes_[i].row() == source_row)
      return index(i, source_index.column());
  }
  return QModelIndex();
}

bool Queue::ContainsSourceRow(int source_row) const {
  for (int i = 0; i < source_indexes_.count(); ++i) {
    if (source_indexes_[i].row() == source_row) return true;
  }
  return false;
}

QModelIndex Queue::mapToSource(const QModelIndex& proxy_index) const {
  if (!proxy_index.isValid()) return QModelIndex();

  return source_indexes_[proxy_index.row()];
}

void Queue::setSourceModel(QAbstractItemModel* source_model) {
  if (sourceModel()) {
    disconnect(sourceModel(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
               this, SLOT(SourceDataChanged(QModelIndex, QModelIndex)));
    disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
               SLOT(SourceLayoutChanged()));
    disconnect(sourceModel(), SIGNAL(layoutChanged()), this,
               SLOT(SourceLayoutChanged()));
  }

  QAbstractProxyModel::setSourceModel(source_model);

  connect(sourceModel(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
          SLOT(SourceDataChanged(QModelIndex, QModelIndex)));
  connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
          SLOT(SourceLayoutChanged()));
  connect(sourceModel(), SIGNAL(layoutChanged()), this,
          SLOT(SourceLayoutChanged()));
}

void Queue::SourceDataChanged(const QModelIndex& top_left,
                              const QModelIndex& bottom_right) {
  for (int row = top_left.row(); row <= bottom_right.row(); ++row) {
    QModelIndex proxy_index = mapFromSource(sourceModel()->index(row, 0));
    if (!proxy_index.isValid()) continue;

    emit dataChanged(proxy_index, proxy_index);
  }
  emit ItemCountChanged(this->ItemCount());
}

void Queue::SourceLayoutChanged() {
  for (int i = 0; i < source_indexes_.count(); ++i) {
    if (!source_indexes_[i].isValid()) {
      beginRemoveRows(QModelIndex(), i, i);
      source_indexes_.removeAt(i);
      endRemoveRows();

      --i;
    }
  }
  emit ItemCountChanged(this->ItemCount());
}

QModelIndex Queue::index(int row, int column, const QModelIndex& parent) const {
  return createIndex(row, column);
}

QModelIndex Queue::parent(const QModelIndex& child) const {
  return QModelIndex();
}

int Queue::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  return source_indexes_.count();
}

int Queue::columnCount(const QModelIndex&) const { return 1; }

QVariant Queue::data(const QModelIndex& proxy_index, int role) const {
  QModelIndex source_index = source_indexes_[proxy_index.row()];

  switch (role) {
    case Playlist::Role_QueuePosition:
      return proxy_index.row();

    case Qt::DisplayRole: {
      const QString artist =
          source_index.sibling(source_index.row(), Playlist::Column_Artist)
              .data()
              .toString();
      const QString title =
          source_index.sibling(source_index.row(), Playlist::Column_Title)
              .data()
              .toString();

      if (artist.isEmpty()) return title;
      return QString(artist + " - " + title);
    }

    default:
      return QVariant();
  }
}

void Queue::ToggleTracks(const QModelIndexList& source_indexes) {
  for (const QModelIndex& source_index : source_indexes) {
    QModelIndex proxy_index = mapFromSource(source_index);
    if (proxy_index.isValid()) {
      // Dequeue the track
      const int row = proxy_index.row();
      beginRemoveRows(QModelIndex(), row, row);
      source_indexes_.removeAt(row);
      endRemoveRows();
    } else {
      // Enqueue the track
      const int row = source_indexes_.count();
      beginInsertRows(QModelIndex(), row, row);
      source_indexes_ << QPersistentModelIndex(source_index);
      endInsertRows();
    }
  }
}

void Queue::InsertFirst(const QModelIndexList& source_indexes) {
  for (const QModelIndex& source_index : source_indexes) {
    QModelIndex proxy_index = mapFromSource(source_index);
    if (proxy_index.isValid()) {
      // Already in the queue, so remove it to be reinserted later
      const int row = proxy_index.row();
      beginRemoveRows(QModelIndex(), row, row);
      source_indexes_.removeAt(row);
      endRemoveRows();
    }
  }

  const int rows = source_indexes.count();
  // Enqueue the tracks at the beginning
  beginInsertRows(QModelIndex(), 0, rows - 1);
  int offset = 0;
  for (const QModelIndex& source_index : source_indexes) {
    source_indexes_.insert(offset, QPersistentModelIndex(source_index));
    offset++;
  }
  endInsertRows();
}

int Queue::PositionOf(const QModelIndex& source_index) const {
  return mapFromSource(source_index).row();
}

bool Queue::is_empty() const { return source_indexes_.isEmpty(); }

int Queue::ItemCount() const { return source_indexes_.length(); }

quint64 Queue::GetTotalLength() const { return total_length_ns_; }

void Queue::UpdateTotalLength() {
  quint64 total = 0;

  for (QPersistentModelIndex row : source_indexes_) {
    int id = row.row();

    Q_ASSERT(playlist_->has_item_at(id));

    quint64 length = playlist_->item_at(id)->Metadata().length_nanosec();
    if (length > 0) total += length;
  }

  total_length_ns_ = total;

  emit TotalLengthChanged(total);
}

void Queue::UpdateSummaryText() {
  QString summary;
  int tracks = this->ItemCount();
  quint64 nanoseconds = this->GetTotalLength();

  // TODO: Make the plurals translatable
  summary += tracks == 1 ? tr("1 track") : tr("%1 tracks").arg(tracks);

  if (nanoseconds)
    summary += " - [ " + Utilities::WordyTimeNanosec(nanoseconds) + " ]";

  emit SummaryTextChanged(summary);
}

void Queue::Clear() {
  if (source_indexes_.isEmpty()) return;

  beginRemoveRows(QModelIndex(), 0, source_indexes_.count() - 1);
  source_indexes_.clear();
  endRemoveRows();
}

void Queue::Move(const QList<int>& proxy_rows, int pos) {
  layoutAboutToBeChanged();
  QList<QPersistentModelIndex> moved_items;

  // Take the items out of the list first, keeping track of whether the
  // insertion point changes
  int offset = 0;
  for (int row : proxy_rows) {
    moved_items << source_indexes_.takeAt(row - offset);
    if (pos != -1 && pos >= row) pos--;
    offset++;
  }

  // Put the items back in
  const int start = pos == -1 ? source_indexes_.count() : pos;
  for (int i = start; i < start + moved_items.count(); ++i) {
    source_indexes_.insert(i, moved_items[i - start]);
  }

  // Update persistent indexes
  for (const QModelIndex& pidx : persistentIndexList()) {
    const int dest_offset = proxy_rows.indexOf(pidx.row());
    if (dest_offset != -1) {
      // This index was moved
      changePersistentIndex(
          pidx, index(start + dest_offset, pidx.column(), QModelIndex()));
    } else {
      int d = 0;
      for (int row : proxy_rows) {
        if (pidx.row() > row) d--;
      }
      if (pidx.row() + d >= start) d += proxy_rows.count();

      changePersistentIndex(
          pidx, index(pidx.row() + d, pidx.column(), QModelIndex()));
    }
  }

  layoutChanged();
}

void Queue::MoveUp(int row) { Move(QList<int>() << row, row - 1); }

void Queue::MoveDown(int row) { Move(QList<int>() << row, row + 2); }

QStringList Queue::mimeTypes() const {
  return QStringList() << kRowsMimetype << Playlist::kRowsMimetype;
}

Qt::DropActions Queue::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

QMimeData* Queue::mimeData(const QModelIndexList& indexes) const {
  QMimeData* data = new QMimeData;

  QList<int> rows;
  for (const QModelIndex& index : indexes) {
    if (index.column() != 0) continue;

    rows << index.row();
  }

  QBuffer buf;
  buf.open(QIODevice::WriteOnly);
  QDataStream stream(&buf);
  stream << rows;
  buf.close();

  data->setData(kRowsMimetype, buf.data());

  return data;
}

bool Queue::dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                         int, const QModelIndex&) {
  if (action == Qt::IgnoreAction) return false;

  if (data->hasFormat(kRowsMimetype)) {
    // Dragged from the queue

    QList<int> proxy_rows;
    QDataStream stream(data->data(kRowsMimetype));
    stream >> proxy_rows;
    // Make sure we take them in order
    std::stable_sort(proxy_rows.begin(), proxy_rows.end());

    Move(proxy_rows, row);
  } else if (data->hasFormat(Playlist::kRowsMimetype)) {
    // Dragged from the playlist

    Playlist* playlist = nullptr;
    QList<int> source_rows;
    QDataStream stream(data->data(Playlist::kRowsMimetype));
    stream.readRawData(reinterpret_cast<char*>(&playlist), sizeof(playlist));
    stream >> source_rows;

    QModelIndexList source_indexes;
    for (int source_row : source_rows) {
      const QModelIndex source_index = sourceModel()->index(source_row, 0);
      const QModelIndex proxy_index = mapFromSource(source_index);
      if (proxy_index.isValid()) {
        // This row was already in the queue, so no need to add it again
        continue;
      }

      source_indexes << source_index;
    }

    if (!source_indexes.isEmpty()) {
      const int insert_point = row == -1 ? source_indexes_.count() : row;
      beginInsertRows(QModelIndex(), insert_point,
                      insert_point + source_indexes.count() - 1);
      for (int i = 0; i < source_indexes.count(); ++i) {
        source_indexes_.insert(insert_point + i, source_indexes[i]);
      }
      endInsertRows();
    }
  }

  return true;
}

Qt::ItemFlags Queue::flags(const QModelIndex& index) const {
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (index.isValid())
    flags |= Qt::ItemIsDragEnabled;
  else
    flags |= Qt::ItemIsDropEnabled;

  return flags;
}

int Queue::PeekNext() const {
  if (source_indexes_.isEmpty()) return -1;
  return source_indexes_.first().row();
}

int Queue::TakeNext() {
  if (source_indexes_.isEmpty()) return -1;

  beginRemoveRows(QModelIndex(), 0, 0);
  int ret = source_indexes_.takeFirst().row();
  endRemoveRows();

  return ret;
}

QVariant Queue::headerData(int section, Qt::Orientation orientation,
                           int role) const {
  return QVariant();
}

void Queue::Remove(QList<int>& proxy_rows) {
  // order the rows
  std::stable_sort(proxy_rows.begin(), proxy_rows.end());

  // reflects immediately changes in the playlist
  layoutAboutToBeChanged();

  int removed_rows = 0;
  for (int row : proxy_rows) {
    // after the first row, the row number needs to be updated
    const int real_row = row - removed_rows;
    beginRemoveRows(QModelIndex(), real_row, real_row);
    source_indexes_.removeAt(real_row);
    endRemoveRows();
    removed_rows++;
  }

  layoutChanged();
}
