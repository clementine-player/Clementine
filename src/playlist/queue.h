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

#ifndef QUEUE_H
#define QUEUE_H

#include "playlist.h"

#include <QAbstractProxyModel>

class Queue : public QAbstractProxyModel {
  Q_OBJECT

public:
  Queue(QObject* parent = 0);

  enum Columns {
    Column_CombinedArtistTitle = Playlist::ColumnCount,

    ColumnCount
  };

  bool is_empty() const;
  int PositionOf(const QModelIndex& source_index) const;

  QModelIndex TakeNext();
  void ToggleTracks(const QModelIndexList& source_indexes);

  // QAbstractProxyModel
  void setSourceModel(QAbstractItemModel* source_model);
  QModelIndex mapFromSource(const QModelIndex& source_index) const;
  QModelIndex mapToSource(const QModelIndex& proxy_index) const;

  // QAbstractItemModel
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex& proxy_index, int role) const;

private slots:
  void SourceDataChanged(const QModelIndex& top_left, const QModelIndex& bottom_right);
  void SourceLayoutChanged();

private:
  QList<QPersistentModelIndex> source_indexes_;
};

#endif // QUEUE_H
