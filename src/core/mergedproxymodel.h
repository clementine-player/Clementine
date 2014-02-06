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

#ifndef MERGEDPROXYMODEL_H
#define MERGEDPROXYMODEL_H

#include <memory>

#include <QAbstractProxyModel>

std::size_t hash_value(const QModelIndex& index);

class MergedProxyModelPrivate;

class MergedProxyModel : public QAbstractProxyModel {
  Q_OBJECT

 public:
  MergedProxyModel(QObject* parent = 0);
  ~MergedProxyModel();

  // Make another model appear as a child of the given item in the source model.
  void AddSubModel(const QModelIndex& source_parent, QAbstractItemModel* submodel);
  void RemoveSubModel(const QModelIndex& source_parent);

  // Find the item in the source model that is the parent of the model
  // containing proxy_index.  If proxy_index is in the source model, then
  // this just returns mapToSource(proxy_index).
  QModelIndex FindSourceParent(const QModelIndex& proxy_index) const;

  // QAbstractItemModel
  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &child) const;
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const;
  bool hasChildren(const QModelIndex &parent) const;
  QMap<int, QVariant> itemData(const QModelIndex &proxyIndex) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList &indexes) const;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
  bool canFetchMore(const QModelIndex &parent) const;
  void fetchMore(const QModelIndex& parent);

  // QAbstractProxyModel
  // Note that these implementations of map{To,From}Source will not always
  // give you an index in sourceModel(), you might get an index in one of the
  // child models instead.
  QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
  QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
  void setSourceModel(QAbstractItemModel *sourceModel);

  // Convenience functions that call map{To,From}Source multiple times.
  QModelIndexList mapFromSource(const QModelIndexList& source_indexes) const;
  QModelIndexList mapToSource(const QModelIndexList& proxy_indexes) const;

 signals:
  void SubModelReset(const QModelIndex& root, QAbstractItemModel* model);

 private slots:
  void SourceModelReset();
  void SubModelReset();

  void RowsAboutToBeInserted(const QModelIndex& source_parent, int start, int end);
  void RowsInserted(const QModelIndex& source_parent, int start, int end);
  void RowsAboutToBeRemoved(const QModelIndex& source_parent, int start, int end);
  void RowsRemoved(const QModelIndex& source_parent, int start, int end);
  void DataChanged(const QModelIndex& top_left, const QModelIndex& bottom_right);

  void LayoutAboutToBeChanged();
  void LayoutChanged();

 private:
  QModelIndex GetActualSourceParent(const QModelIndex& source_parent,
                                    QAbstractItemModel* model) const;
  QAbstractItemModel* GetModel(const QModelIndex& source_index) const;
  void DeleteAllMappings();
  bool IsKnownModel(const QAbstractItemModel* model) const;


  QMap<QAbstractItemModel*, QPersistentModelIndex> merge_points_;
  QAbstractItemModel* resetting_model_;

  QMap<QAbstractItemModel*, QModelIndex> old_merge_points_;

  std::unique_ptr<MergedProxyModelPrivate> p_;
};

#endif // MERGEDPROXYMODEL_H
