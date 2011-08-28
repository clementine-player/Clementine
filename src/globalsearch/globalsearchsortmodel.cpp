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

  int ret = 0;

#define CompareInt(field) \
  if (r1.field < r2.field) return true; \
  if (r1.field > r2.field) return false

#define CompareString(field) \
  ret = QString::localeAwareCompare(r1.metadata_.field(), r2.metadata_.field()); \
  if (ret < 0) return true; \
  if (ret > 0) return false


  // Compare match quality and types first
  CompareInt(match_quality_);
  CompareInt(type_);

  // Then compare title, artist and album
  switch (r1.type_) {
  case SearchProvider::Result::Type_Track:
    CompareString(title);
    // fallthrough
  case SearchProvider::Result::Type_Album:
    CompareString(artist);
    CompareString(album);
    break;
  }

  return false;

#undef CompareInt
#undef CompareString
}
