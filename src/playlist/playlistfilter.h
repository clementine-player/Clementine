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

#ifndef PLAYLISTFILTER_H
#define PLAYLISTFILTER_H

#include <QScopedPointer>
#include <QSortFilterProxyModel>

#include "playlist.h"

#include <QSet>

class FilterTree;

class PlaylistFilter : public QSortFilterProxyModel {
  Q_OBJECT

 public:
  PlaylistFilter(QObject* parent = 0);
  ~PlaylistFilter();

  // QAbstractItemModel
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  // QSortFilterProxyModel
  // public so Playlist::NextVirtualIndex and friends can get at it
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

 private:
  // Mutable because they're modified from filterAcceptsRow() const
  mutable QScopedPointer<FilterTree> filter_tree_;
  mutable uint query_hash_;

  QMap<QString, int> column_names_;
  QSet<int> numerical_columns_;
};

#endif  // PLAYLISTFILTER_H
