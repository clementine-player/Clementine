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

#include "globalsearchsortmodel.h"

#include "core/logging.h"
#include "globalsearchmodel.h"
#include "searchprovider.h"

GlobalSearchSortModel::GlobalSearchSortModel(QObject* parent)
    : QSortFilterProxyModel(parent) {}

bool GlobalSearchSortModel::lessThan(const QModelIndex& left,
                                     const QModelIndex& right) const {
  // Compare the provider sort index first.
  const int index_left =
      left.data(GlobalSearchModel::Role_ProviderIndex).toInt();
  const int index_right =
      right.data(GlobalSearchModel::Role_ProviderIndex).toInt();
  if (index_left < index_right) return true;
  if (index_left > index_right) return false;

  // Dividers always go first
  if (left.data(LibraryModel::Role_IsDivider).toBool()) return true;
  if (right.data(LibraryModel::Role_IsDivider).toBool()) return false;

  // Containers go before songs if they're at the same level
  const bool left_is_container =
      left.data(LibraryModel::Role_ContainerType).isValid();
  const bool right_is_container =
      right.data(LibraryModel::Role_ContainerType).isValid();
  if (left_is_container && !right_is_container) return true;
  if (right_is_container && !left_is_container) return false;

  // Containers get sorted on their sort text.
  if (left_is_container) {
    return QString::localeAwareCompare(
               left.data(LibraryModel::Role_SortText).toString(),
               right.data(LibraryModel::Role_SortText).toString()) < 0;
  }

  // Otherwise we're comparing songs.  Sort by disc, track, then title.
  const SearchProvider::Result r1 =
      left.data(GlobalSearchModel::Role_Result).value<SearchProvider::Result>();
  const SearchProvider::Result r2 = right.data(GlobalSearchModel::Role_Result)
                                        .value<SearchProvider::Result>();

#define CompareInt(field)                                       \
  if (r1.metadata_.field() < r2.metadata_.field()) return true; \
  if (r1.metadata_.field() > r2.metadata_.field()) return false

  int ret = 0;

#define CompareString(field)                                                   \
  ret =                                                                        \
      QString::localeAwareCompare(r1.metadata_.field(), r2.metadata_.field()); \
  if (ret < 0) return true;                                                    \
  if (ret > 0) return false

  CompareInt(disc);
  CompareInt(track);
  CompareString(title);

  return false;

#undef CompareInt
#undef CompareString
}
