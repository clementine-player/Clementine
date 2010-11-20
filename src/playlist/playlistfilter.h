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

#include <QSortFilterProxyModel>

#include "playlist.h"

#include <QSet>

class PlaylistFilter : public QSortFilterProxyModel {
  Q_OBJECT

public:
  PlaylistFilter(QObject* parent = 0);

  // QAbstractItemModel
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  // QSortFilterProxyModel
  // public so Playlist::NextVirtualIndex and friends can get at it
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
  struct SearchTerm {
    SearchTerm(const QString& value = QString(), int column = -1, bool exact = false)
      : value_(value), column_(column), exact_(exact) {}

    QString value_;
    int column_;
    bool exact_;
  };

  // Mutable because they're modified from filterAcceptsRow() const
  mutable QList<SearchTerm> query_cache_;
  mutable uint query_hash_;

  QMap<QString, int> column_names_;
  QSet<int> exact_columns_;
};

#endif // PLAYLISTFILTER_H
