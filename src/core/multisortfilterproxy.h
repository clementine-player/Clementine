/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef CORE_MULTISORTFILTERPROXY_H_
#define CORE_MULTISORTFILTERPROXY_H_

#include <QSortFilterProxyModel>

class MultiSortFilterProxy : public QSortFilterProxyModel {
 public:
  explicit MultiSortFilterProxy(QObject* parent = nullptr);

  void AddSortSpec(int role, Qt::SortOrder order = Qt::AscendingOrder);

 protected:
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

 private:
  int Compare(const QVariant& left, const QVariant& right) const;

  typedef QPair<int, Qt::SortOrder> SortSpec;
  QList<SortSpec> sorting_;
};

#endif  // CORE_MULTISORTFILTERPROXY_H_
