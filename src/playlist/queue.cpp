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

#include "queue.h"

Queue::Queue(QObject* parent)
  : QAbstractProxyModel(parent)
{
}

QModelIndex Queue::mapFromSource(const QModelIndex& source_index) const {
  if (!source_index.isValid())
    return QModelIndex();

  const int source_row = source_index.row();
  for (int i=0 ; i<source_indexes_.count() ; ++i) {
    if (source_indexes_[i].row() == source_row)
      return index(i, source_index.column());
  }
  return QModelIndex();
}

QModelIndex Queue::mapToSource(const QModelIndex& proxy_index) const {
  return source_indexes_[proxy_index.row()];
}

void Queue::setSourceModel(QAbstractItemModel* source_model) {
  if (sourceModel()) {
    disconnect(sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this, SLOT(SourceDataChanged(QModelIndex,QModelIndex)));
    disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
               this, SLOT(SourceLayoutChanged()));
    disconnect(sourceModel(), SIGNAL(layoutChanged()),
               this, SLOT(SourceLayoutChanged()));
  }

  QAbstractProxyModel::setSourceModel(source_model);

  connect(sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(SourceDataChanged(QModelIndex,QModelIndex)));
  connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
          this, SLOT(SourceLayoutChanged()));
  connect(sourceModel(), SIGNAL(layoutChanged()),
          this, SLOT(SourceLayoutChanged()));
}

void Queue::SourceDataChanged(const QModelIndex& top_left,
                              const QModelIndex& bottom_right) {
  const int last_col = ColumnCount - 1;

  for (int row = top_left.row() ; row <= bottom_right.row() ; ++row) {
    QModelIndex proxy_index = mapFromSource(sourceModel()->index(row, 0));
    if (!proxy_index.isValid())
      continue;

    emit dataChanged(proxy_index, proxy_index.sibling(proxy_index.row(), last_col));
  }
}

void Queue::SourceLayoutChanged() {
  for (int i=0 ; i<source_indexes_.count() ; ++i) {
    if (!source_indexes_[i].isValid()) {
      beginRemoveRows(QModelIndex(), i, i);
      source_indexes_.removeAt(i);
      endRemoveRows();

      --i;
    }
  }
}

QModelIndex Queue::index(int row, int column, const QModelIndex& parent) const {
  return createIndex(row, column);
}

QModelIndex Queue::parent(const QModelIndex &child) const {
  return QModelIndex();
}

int Queue::rowCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;
  return source_indexes_.count();
}

int Queue::columnCount(const QModelIndex &parent) const {
  return ColumnCount;
}

QVariant Queue::data(const QModelIndex& proxy_index, int role) const {
  QModelIndex source_index = source_indexes_[proxy_index.row()];

  switch (role) {
    case Playlist::Role_QueuePosition:
      return proxy_index.row();

    case Qt::DisplayRole:
      if (proxy_index.column() == Column_CombinedArtistTitle) {
        const QString artist = source_index.sibling(source_index.row(), Playlist::Column_Artist).data().toString();
        const QString title = source_index.sibling(source_index.row(), Playlist::Column_Title).data().toString();

        if (artist.isEmpty())
          return title;
        return artist + " - " + title;
      }
      // fallthrough

    default:
      return source_index.data(role);
  }
}

void Queue::ToggleTracks(const QModelIndexList &source_indexes) {
  foreach (const QModelIndex& source_index, source_indexes) {
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

int Queue::PositionOf(const QModelIndex& source_index) const {
  return mapFromSource(source_index).row();
}

bool Queue::is_empty() const {
  return source_indexes_.isEmpty();
}

void Queue::Clear() {
  if (source_indexes_.isEmpty())
    return;

  beginRemoveRows(QModelIndex(), 0, source_indexes_.count()-1);
  source_indexes_.clear();
  endRemoveRows();
}
