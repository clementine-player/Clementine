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

#include "mergedproxymodel.h"

std::size_t hash_value(const QModelIndex& index) {
  return qHash(index);
}

MergedProxyModel::MergedProxyModel(QObject* parent)
  : QAbstractProxyModel(parent)
{
}

MergedProxyModel::~MergedProxyModel() {
  DeleteAllMappings();
}

void MergedProxyModel::DeleteAllMappings() {
  MappingContainer::index<tag_by_pointer>::type::iterator begin =
      mappings_.get<tag_by_pointer>().begin();
  MappingContainer::index<tag_by_pointer>::type::iterator end =
      mappings_.get<tag_by_pointer>().end();
  qDeleteAll(begin, end);
}

void MergedProxyModel::AddSubModel(const QModelIndex& source_parent,
                                   const QAbstractItemModel* submodel) {
  merge_points_.insert(submodel, source_parent);

  connect(submodel, SIGNAL(modelReset()), this, SLOT(SubModelReset()));
  connect(submodel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
          this, SLOT(RowsAboutToBeInserted(QModelIndex,int,int)));
  connect(submodel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
          this, SLOT(RowsAboutToBeRemoved(QModelIndex,int,int)));
  connect(submodel, SIGNAL(rowsInserted(QModelIndex,int,int)),
          this, SLOT(RowsInserted(QModelIndex,int,int)));
  connect(submodel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
          this, SLOT(RowsRemoved(QModelIndex,int,int)));
}

void MergedProxyModel::setSourceModel(QAbstractItemModel* source_model) {
  if (sourceModel()) {
    disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(SourceModelReset()));
    disconnect(sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
               this, SLOT(RowsAboutToBeInserted(QModelIndex,int,int)));
    disconnect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
               this, SLOT(RowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(RowsInserted(QModelIndex,int,int)));
    disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
               this, SLOT(RowsRemoved(QModelIndex,int,int)));
  }

  QAbstractProxyModel::setSourceModel(source_model);

  connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(SourceModelReset()));
  connect(sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
          this, SLOT(RowsAboutToBeInserted(QModelIndex,int,int)));
  connect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
          this, SLOT(RowsAboutToBeRemoved(QModelIndex,int,int)));
  connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
          this, SLOT(RowsInserted(QModelIndex,int,int)));
  connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
          this, SLOT(RowsRemoved(QModelIndex,int,int)));
}

void MergedProxyModel::SourceModelReset() {
  // Delete all mappings
  DeleteAllMappings();

  // Clear the containers
  mappings_.clear();
  merge_points_.clear();

  // Reset the proxy
  reset();
}

void MergedProxyModel::SubModelReset() {
  const QAbstractItemModel* submodel = static_cast<const QAbstractItemModel*>(sender());

  // Delete all the mappings that reference the submodel
  MappingContainer::index<tag_by_pointer>::type::iterator it =
      mappings_.get<tag_by_pointer>().begin();
  MappingContainer::index<tag_by_pointer>::type::iterator end =
      mappings_.get<tag_by_pointer>().end();

  while (it != end) {
    if ((*it)->source_index.model() == submodel) {
      delete *it;
      it = mappings_.get<tag_by_pointer>().erase(it);
    } else {
      ++it;
    }
  }

  // Reset the proxy
  reset();
}

QModelIndex MergedProxyModel::GetActualSourceParent(const QModelIndex& source_parent,
                                                    const QAbstractItemModel* model) const {
  if (!source_parent.isValid() && model != sourceModel())
    return merge_points_.value(model);
  return source_parent;
}

void MergedProxyModel::RowsAboutToBeInserted(const QModelIndex& source_parent,
                                             int start, int end) {
  beginInsertRows(mapFromSource(GetActualSourceParent(
      source_parent, static_cast<const QAbstractItemModel*>(sender()))),
      start, end);
}

void MergedProxyModel::RowsInserted(const QModelIndex&, int, int) {
  endInsertRows();
}

void MergedProxyModel::RowsAboutToBeRemoved(const QModelIndex& source_parent,
                                            int start, int end) {
  beginRemoveRows(mapFromSource(GetActualSourceParent(
      source_parent, static_cast<const QAbstractItemModel*>(sender()))),
      start, end);
}

void MergedProxyModel::RowsRemoved(const QModelIndex&, int, int) {
  endRemoveRows();
}

QModelIndex MergedProxyModel::mapToSource(const QModelIndex& proxy_index) const {
  if (!proxy_index.isValid())
    return QModelIndex();

  Mapping* mapping = static_cast<Mapping*>(proxy_index.internalPointer());
  return mapping->source_index;
}

QModelIndex MergedProxyModel::mapFromSource(const QModelIndex& source_index) const {
  if (!source_index.isValid())
    return QModelIndex();

  // Add a mapping if we don't have one already
  MappingContainer::index<tag_by_source>::type::iterator it =
      mappings_.get<tag_by_source>().find(source_index);
  Mapping* mapping;
  if (it != mappings_.get<tag_by_source>().end()) {
    mapping = *it;
  } else {
    mapping = new Mapping(source_index);
    const_cast<MergedProxyModel*>(this)->mappings_.insert(mapping);
  }

  return createIndex(source_index.row(), source_index.column(), mapping);
}

QModelIndex MergedProxyModel::index(int row, int column, const QModelIndex &parent) const {
  QModelIndex source_index;

  if (!parent.isValid()) {
    source_index = sourceModel()->index(row, column, QModelIndex());
  } else {
    QModelIndex source_parent = mapToSource(parent);
    const QAbstractItemModel* child_model = merge_points_.key(source_parent);

    if (child_model)
      source_index = child_model->index(row, column, QModelIndex());
    else
      source_index = source_parent.model()->index(row, column, source_parent);
  }

  return mapFromSource(source_index);
}

QModelIndex MergedProxyModel::parent(const QModelIndex &child) const {
  QModelIndex source_child = mapToSource(child);
  if (source_child.model() == sourceModel())
    return mapFromSource(source_child.parent());

  if (!source_child.parent().isValid())
    return mapFromSource(merge_points_.value(source_child.model()));
  return mapFromSource(source_child.parent());
}

int MergedProxyModel::rowCount(const QModelIndex &parent) const {
  if (!parent.isValid())
    return sourceModel()->rowCount(QModelIndex());

  QModelIndex source_parent = mapToSource(parent);
  const QAbstractItemModel* child_model = merge_points_.key(source_parent);
  if (child_model) {
    // Query the source model but disregard what it says, so it gets a chance
    // to lazy load
    source_parent.model()->rowCount(source_parent);

    return child_model->rowCount(QModelIndex());
  }

  return source_parent.model()->rowCount(source_parent);
}

int MergedProxyModel::columnCount(const QModelIndex &parent) const {
  if (!parent.isValid())
    return sourceModel()->columnCount(QModelIndex());

  QModelIndex source_parent = mapToSource(parent);
  const QAbstractItemModel* child_model = merge_points_.key(source_parent);
  if (child_model)
    return child_model->columnCount(QModelIndex());
  return source_parent.model()->columnCount(source_parent);
}

bool MergedProxyModel::hasChildren(const QModelIndex &parent) const {
  if (!parent.isValid())
    return sourceModel()->hasChildren(QModelIndex());

  QModelIndex source_parent = mapToSource(parent);
  const QAbstractItemModel* child_model = merge_points_.key(source_parent);

  if (child_model)
    return child_model->hasChildren(QModelIndex()) ||
           source_parent.model()->hasChildren(source_parent);
  return source_parent.model()->hasChildren(source_parent);
}

QVariant MergedProxyModel::data(const QModelIndex &proxyIndex, int role) const {
  QModelIndex source_index = mapToSource(proxyIndex);
  return source_index.model()->data(source_index, role);
}
