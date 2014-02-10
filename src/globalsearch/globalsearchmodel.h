/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef GLOBALSEARCHMODEL_H
#define GLOBALSEARCHMODEL_H

#include "searchprovider.h"
#include "library/librarymodel.h"

#include <QStandardItemModel>

class GlobalSearch;

class QSortFilterProxyModel;

class GlobalSearchModel : public QStandardItemModel {
  Q_OBJECT

 public:
  GlobalSearchModel(GlobalSearch* engine, QObject* parent = nullptr);

  enum Role {
    Role_Result = LibraryModel::LastRole,
    Role_LazyLoadingArt,
    Role_ProviderIndex,
    LastRole
  };

  struct ContainerKey {
    int provider_index_;
    QString group_[3];
  };

  void set_proxy(QSortFilterProxyModel* proxy) { proxy_ = proxy; }
  void set_use_pretty_covers(bool pretty) { use_pretty_covers_ = pretty; }
  void set_provider_order(const QStringList& provider_order) {
    provider_order_ = provider_order;
  }
  void SetGroupBy(const LibraryModel::Grouping& grouping, bool regroup_now);

  void Clear();

  SearchProvider::ResultList GetChildResults(const QModelIndexList& indexes)
      const;
  SearchProvider::ResultList GetChildResults(const QList<QStandardItem*>& items)
      const;

  // QAbstractItemModel
  QMimeData* mimeData(const QModelIndexList& indexes) const;

 public slots:
  void AddResults(const SearchProvider::ResultList& results);

 private:
  QStandardItem* BuildContainers(const Song& metadata, QStandardItem* parent,
                                 ContainerKey* key, int level = 0);
  void GetChildResults(const QStandardItem* item,
                       SearchProvider::ResultList* results,
                       QSet<const QStandardItem*>* visited) const;

 private:
  GlobalSearch* engine_;
  QSortFilterProxyModel* proxy_;

  LibraryModel::Grouping group_by_;

  QMap<SearchProvider*, int> provider_sort_indices_;
  int next_provider_sort_index_;
  QMap<ContainerKey, QStandardItem*> containers_;

  QStringList provider_order_;
  bool use_pretty_covers_;
  QIcon artist_icon_;
  QIcon album_icon_;
  QPixmap no_cover_icon_;
};

inline uint qHash(const GlobalSearchModel::ContainerKey& key) {
  return qHash(key.provider_index_) ^ qHash(key.group_[0]) ^
         qHash(key.group_[1]) ^ qHash(key.group_[2]);
}

inline bool operator<(const GlobalSearchModel::ContainerKey& left,
                      const GlobalSearchModel::ContainerKey& right) {
#define CMP(field)                           \
  if (left.field < right.field) return true; \
  if (left.field > right.field) return false

  CMP(provider_index_);
  CMP(group_[0]);
  CMP(group_[1]);
  CMP(group_[2]);
  return false;

#undef CMP
}

#endif  // GLOBALSEARCHMODEL_H
