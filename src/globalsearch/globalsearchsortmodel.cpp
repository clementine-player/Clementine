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

#include "globalsearchwidget.h"
#include "globalsearchsortmodel.h"
#include "searchprovider.h"
#include "core/logging.h"

GlobalSearchSortModel::GlobalSearchSortModel(QObject* parent)
  : QSortFilterProxyModel(parent)
{
}

bool GlobalSearchSortModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
  const SearchProvider::Result r1 = left.data(GlobalSearchWidget::Role_Result)
      .value<SearchProvider::Result>();
  const SearchProvider::Result r2 = right.data(GlobalSearchWidget::Role_Result)
      .value<SearchProvider::Result>();

  // Compare types first
  if (r1.type_ < r2.type_) return true;
  if (r1.type_ > r2.type_) return false;

  int ret = 0;
  switch (r1.type_) {
  case SearchProvider::Result::Type_Track:
    ret = QString::localeAwareCompare(r1.metadata_.title(), r2.metadata_.title());
    if (ret < 0) return true;
    if (ret > 0) return false;

    ret = QString::localeAwareCompare(r1.metadata_.artist(), r2.metadata_.artist());
    if (ret < 0) return true;
    if (ret > 0) return false;

    ret = QString::localeAwareCompare(r1.metadata_.album(), r2.metadata_.album());
    if (ret < 0) return true;
    if (ret > 0) return false;

    break;

  case SearchProvider::Result::Type_Album:
    ret = QString::localeAwareCompare(r1.metadata_.artist(), r2.metadata_.artist());
    if (ret < 0) return true;
    if (ret > 0) return false;

    ret = QString::localeAwareCompare(r1.metadata_.album(), r2.metadata_.album());
    if (ret < 0) return true;
    if (ret > 0) return false;

    break;
  }

  return false;
}
