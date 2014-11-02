/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
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

#include "mergedproxymodel.h"
#include "core/logging.h"

#include <QStringList>

#include <limits>

// boost::multi_index still relies on these being in the global namespace.
using std::placeholders::_1;
using std::placeholders::_2;

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>

using boost::multi_index::hashed_unique;
using boost::multi_index::identity;
using boost::multi_index::indexed_by;
using boost::multi_index::member;
using boost::multi_index::multi_index_container;
using boost::multi_index::ordered_unique;
using boost::multi_index::tag;

std::size_t hash_value(const QModelIndex& index) { return qHash(index); }

namespace {

struct Mapping {
  explicit Mapping(const QModelIndex& _source_index) : source_index(_source_index) {}

  QModelIndex source_index;
};

struct tag_by_source {};
struct tag_by_pointer {};

}  // namespace

class MergedProxyModelPrivate {
 private:
  typedef multi_index_container<
      Mapping*,
      indexed_by<
          hashed_unique<tag<tag_by_source>,
                        member<Mapping, QModelIndex, &Mapping::source_index> >,
          ordered_unique<tag<tag_by_pointer>, identity<Mapping*> > > >
      MappingContainer;

 public:
  MappingContainer mappings_;
};

MergedProxyModel::MergedProxyModel(QObject* parent)
    : QAbstractProxyModel(parent),
      resetting_model_(nullptr),
      p_(new MergedProxyModelPrivate) {}

MergedProxyModel::~MergedProxyModel() { DeleteAllMappings(); }

void MergedProxyModel::DeleteAllMappings() {
  const auto& begin = p_->mappings_.get<tag_by_pointer>().begin();
  const auto& end = p_->mappings_.get<tag_by_pointer>().end();
  qDeleteAll(begin, end);
}

void MergedProxyModel::AddSubModel(const QModelIndex& source_parent,
                                   QAbstractItemModel* submodel) {
  connect(submodel, SIGNAL(modelReset()), this, SLOT(SubModelReset()));
  connect(submodel, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this,
          SLOT(RowsAboutToBeInserted(QModelIndex, int, int)));
  connect(submodel, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this,
          SLOT(RowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(submodel, SIGNAL(rowsInserted(QModelIndex, int, int)), this,
          SLOT(RowsInserted(QModelIndex, int, int)));
  connect(submodel, SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
          SLOT(RowsRemoved(QModelIndex, int, int)));
  connect(submodel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
          SLOT(DataChanged(QModelIndex, QModelIndex)));

  QModelIndex proxy_parent = mapFromSource(source_parent);
  const int rows = submodel->rowCount();

  if (rows) beginInsertRows(proxy_parent, 0, rows - 1);

  merge_points_.insert(submodel, source_parent);

  if (rows) endInsertRows();
}

void MergedProxyModel::RemoveSubModel(const QModelIndex& source_parent) {
  // Find the submodel that the parent corresponded to
  QAbstractItemModel* submodel = merge_points_.key(source_parent);
  merge_points_.remove(submodel);

  // The submodel might have been deleted already so we must be careful not
  // to dereference it.

  // Remove all the children of the item that got deleted
  QModelIndex proxy_parent = mapFromSource(source_parent);

  // We can't know how many children it had, since we can't dereference it
  resetting_model_ = submodel;
  beginRemoveRows(proxy_parent, 0, std::numeric_limits<int>::max() - 1);
  endRemoveRows();
  resetting_model_ = nullptr;

  // Delete all the mappings that reference the submodel
  auto it = p_->mappings_.get<tag_by_pointer>().begin();
  auto end = p_->mappings_.get<tag_by_pointer>().end();
  while (it != end) {
    if ((*it)->source_index.model() == submodel) {
      delete *it;
      it = p_->mappings_.get<tag_by_pointer>().erase(it);
    } else {
      ++it;
    }
  }
}

void MergedProxyModel::setSourceModel(QAbstractItemModel* source_model) {
  if (sourceModel()) {
    disconnect(sourceModel(), SIGNAL(modelReset()), this,
               SLOT(SourceModelReset()));
    disconnect(sourceModel(),
               SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this,
               SLOT(RowsAboutToBeInserted(QModelIndex, int, int)));
    disconnect(sourceModel(),
               SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this,
               SLOT(RowsAboutToBeRemoved(QModelIndex, int, int)));
    disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex, int, int)), this,
               SLOT(RowsInserted(QModelIndex, int, int)));
    disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
               SLOT(RowsRemoved(QModelIndex, int, int)));
    disconnect(sourceModel(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
               this, SLOT(DataChanged(QModelIndex, QModelIndex)));
    disconnect(sourceModel(), SIGNAL(layoutAboutToBeChanged()), this,
               SLOT(LayoutAboutToBeChanged()));
    disconnect(sourceModel(), SIGNAL(layoutChanged()), this,
               SLOT(LayoutChanged()));
  }

  QAbstractProxyModel::setSourceModel(source_model);

  connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(SourceModelReset()));
  connect(sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
          this, SLOT(RowsAboutToBeInserted(QModelIndex, int, int)));
  connect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
          this, SLOT(RowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex, int, int)), this,
          SLOT(RowsInserted(QModelIndex, int, int)));
  connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
          SLOT(RowsRemoved(QModelIndex, int, int)));
  connect(sourceModel(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
          SLOT(DataChanged(QModelIndex, QModelIndex)));
  connect(sourceModel(), SIGNAL(layoutAboutToBeChanged()), this,
          SLOT(LayoutAboutToBeChanged()));
  connect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(LayoutChanged()));
}

void MergedProxyModel::SourceModelReset() {
  // Delete all mappings
  DeleteAllMappings();

  // Clear the containers
  p_->mappings_.clear();
  merge_points_.clear();

  // Reset the proxy
  reset();
}

void MergedProxyModel::SubModelReset() {
  QAbstractItemModel* submodel = static_cast<QAbstractItemModel*>(sender());

  // TODO(David Sansome): When we require Qt 4.6, use beginResetModel() and endResetModel()
  // in LibraryModel and catch those here - that will let us do away with this
  // std::numeric_limits<int>::max() hack.

  // Remove all the children of the item that got deleted
  QModelIndex source_parent = merge_points_.value(submodel);
  QModelIndex proxy_parent = mapFromSource(source_parent);

  // We can't know how many children it had, since it's already disappeared...
  resetting_model_ = submodel;
  beginRemoveRows(proxy_parent, 0, std::numeric_limits<int>::max() - 1);
  endRemoveRows();
  resetting_model_ = nullptr;

  // Delete all the mappings that reference the submodel
  auto it = p_->mappings_.get<tag_by_pointer>().begin();
  auto end = p_->mappings_.get<tag_by_pointer>().end();
  while (it != end) {
    if ((*it)->source_index.model() == submodel) {
      delete *it;
      it = p_->mappings_.get<tag_by_pointer>().erase(it);
    } else {
      ++it;
    }
  }

  // "Insert" items from the newly reset submodel
  int count = submodel->rowCount();
  if (count) {
    beginInsertRows(proxy_parent, 0, count - 1);
    endInsertRows();
  }

  emit SubModelReset(proxy_parent, submodel);
}

QModelIndex MergedProxyModel::GetActualSourceParent(
    const QModelIndex& source_parent, QAbstractItemModel* model) const {
  if (!source_parent.isValid() && model != sourceModel())
    return merge_points_.value(model);
  return source_parent;
}

void MergedProxyModel::RowsAboutToBeInserted(const QModelIndex& source_parent,
                                             int start, int end) {
  beginInsertRows(
      mapFromSource(GetActualSourceParent(
          source_parent, static_cast<QAbstractItemModel*>(sender()))),
      start, end);
}

void MergedProxyModel::RowsInserted(const QModelIndex&, int, int) {
  endInsertRows();
}

void MergedProxyModel::RowsAboutToBeRemoved(const QModelIndex& source_parent,
                                            int start, int end) {
  beginRemoveRows(
      mapFromSource(GetActualSourceParent(
          source_parent, static_cast<QAbstractItemModel*>(sender()))),
      start, end);
}

void MergedProxyModel::RowsRemoved(const QModelIndex&, int, int) {
  endRemoveRows();
}

QModelIndex MergedProxyModel::mapToSource(const QModelIndex& proxy_index)
    const {
  if (!proxy_index.isValid()) return QModelIndex();

  Mapping* mapping = static_cast<Mapping*>(proxy_index.internalPointer());
  if (p_->mappings_.get<tag_by_pointer>().find(mapping) ==
      p_->mappings_.get<tag_by_pointer>().end())
    return QModelIndex();
  if (mapping->source_index.model() == resetting_model_) return QModelIndex();

  return mapping->source_index;
}

QModelIndex MergedProxyModel::mapFromSource(const QModelIndex& source_index)
    const {
  if (!source_index.isValid()) return QModelIndex();
  if (source_index.model() == resetting_model_) return QModelIndex();

  // Add a mapping if we don't have one already
  const auto& it = p_->mappings_.get<tag_by_source>().find(source_index);
  Mapping* mapping;
  if (it != p_->mappings_.get<tag_by_source>().end()) {
    mapping = *it;
  } else {
    mapping = new Mapping(source_index);
    const_cast<MergedProxyModel*>(this)->p_->mappings_.insert(mapping);
  }

  return createIndex(source_index.row(), source_index.column(), mapping);
}

QModelIndex MergedProxyModel::index(int row, int column,
                                    const QModelIndex& parent) const {
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

QModelIndex MergedProxyModel::parent(const QModelIndex& child) const {
  QModelIndex source_child = mapToSource(child);
  if (source_child.model() == sourceModel())
    return mapFromSource(source_child.parent());

  if (!IsKnownModel(source_child.model())) return QModelIndex();

  if (!source_child.parent().isValid())
    return mapFromSource(merge_points_.value(GetModel(source_child)));
  return mapFromSource(source_child.parent());
}

int MergedProxyModel::rowCount(const QModelIndex& parent) const {
  if (!parent.isValid()) return sourceModel()->rowCount(QModelIndex());

  QModelIndex source_parent = mapToSource(parent);
  if (!IsKnownModel(source_parent.model())) return 0;

  const QAbstractItemModel* child_model = merge_points_.key(source_parent);
  if (child_model) {
    // Query the source model but disregard what it says, so it gets a chance
    // to lazy load
    source_parent.model()->rowCount(source_parent);

    return child_model->rowCount(QModelIndex());
  }

  return source_parent.model()->rowCount(source_parent);
}

int MergedProxyModel::columnCount(const QModelIndex& parent) const {
  if (!parent.isValid()) return sourceModel()->columnCount(QModelIndex());

  QModelIndex source_parent = mapToSource(parent);
  if (!IsKnownModel(source_parent.model())) return 0;

  const QAbstractItemModel* child_model = merge_points_.key(source_parent);
  if (child_model) return child_model->columnCount(QModelIndex());
  return source_parent.model()->columnCount(source_parent);
}

bool MergedProxyModel::hasChildren(const QModelIndex& parent) const {
  if (!parent.isValid()) return sourceModel()->hasChildren(QModelIndex());

  QModelIndex source_parent = mapToSource(parent);
  if (!IsKnownModel(source_parent.model())) return false;

  const QAbstractItemModel* child_model = merge_points_.key(source_parent);

  if (child_model)
    return child_model->hasChildren(QModelIndex()) ||
           source_parent.model()->hasChildren(source_parent);
  return source_parent.model()->hasChildren(source_parent);
}

QVariant MergedProxyModel::data(const QModelIndex& proxyIndex, int role) const {
  QModelIndex source_index = mapToSource(proxyIndex);
  if (!IsKnownModel(source_index.model())) return QVariant();

  return source_index.model()->data(source_index, role);
}

QMap<int, QVariant> MergedProxyModel::itemData(const QModelIndex& proxy_index)
    const {
  QModelIndex source_index = mapToSource(proxy_index);

  if (!source_index.isValid()) return sourceModel()->itemData(QModelIndex());
  return source_index.model()->itemData(source_index);
}

Qt::ItemFlags MergedProxyModel::flags(const QModelIndex& index) const {
  QModelIndex source_index = mapToSource(index);

  if (!source_index.isValid()) return sourceModel()->flags(QModelIndex());
  return source_index.model()->flags(source_index);
}

bool MergedProxyModel::setData(const QModelIndex& index, const QVariant& value,
                               int role) {
  QModelIndex source_index = mapToSource(index);

  if (!source_index.isValid())
    return sourceModel()->setData(index, value, role);
  return GetModel(index)->setData(index, value, role);
}

QStringList MergedProxyModel::mimeTypes() const {
  QStringList ret;
  ret << sourceModel()->mimeTypes();

  for (const QAbstractItemModel* model : merge_points_.keys()) {
    ret << model->mimeTypes();
  }

  return ret;
}

QMimeData* MergedProxyModel::mimeData(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) return 0;

  // Only ask the first index's model
  const QAbstractItemModel* model = mapToSource(indexes[0]).model();
  if (!model) {
    return 0;
  }

  // Only ask about the indexes that are actually in that model
  QModelIndexList indexes_in_model;

  for (const QModelIndex& proxy_index : indexes) {
    QModelIndex source_index = mapToSource(proxy_index);
    if (source_index.model() != model) continue;
    indexes_in_model << source_index;
  }

  return model->mimeData(indexes_in_model);
}

bool MergedProxyModel::dropMimeData(const QMimeData* data,
                                    Qt::DropAction action, int row, int column,
                                    const QModelIndex& parent) {
  if (!parent.isValid()) {
    return false;
  }

  return sourceModel()->dropMimeData(data, action, row, column, parent);
}

QModelIndex MergedProxyModel::FindSourceParent(const QModelIndex& proxy_index)
    const {
  if (!proxy_index.isValid()) return QModelIndex();

  QModelIndex source_index = mapToSource(proxy_index);
  if (source_index.model() == sourceModel()) return source_index;
  return merge_points_.value(GetModel(source_index));
}

bool MergedProxyModel::canFetchMore(const QModelIndex& parent) const {
  QModelIndex source_index = mapToSource(parent);

  if (!source_index.isValid())
    return sourceModel()->canFetchMore(QModelIndex());
  return source_index.model()->canFetchMore(source_index);
}

void MergedProxyModel::fetchMore(const QModelIndex& parent) {
  QModelIndex source_index = mapToSource(parent);

  if (!source_index.isValid())
    sourceModel()->fetchMore(QModelIndex());
  else
    GetModel(source_index)->fetchMore(source_index);
}

QAbstractItemModel* MergedProxyModel::GetModel(const QModelIndex& source_index)
    const {
  // This is essentially const_cast<QAbstractItemModel*>(source_index.model()),
  // but without the const_cast
  const QAbstractItemModel* const_model = source_index.model();
  if (const_model == sourceModel()) return sourceModel();
  for (QAbstractItemModel* submodel : merge_points_.keys()) {
    if (submodel == const_model) return submodel;
  }
  return nullptr;
}

void MergedProxyModel::DataChanged(const QModelIndex& top_left,
                                   const QModelIndex& bottom_right) {
  emit dataChanged(mapFromSource(top_left), mapFromSource(bottom_right));
}

void MergedProxyModel::LayoutAboutToBeChanged() {
  old_merge_points_.clear();
  for (QAbstractItemModel* key : merge_points_.keys()) {
    old_merge_points_[key] = merge_points_.value(key);
  }
}

void MergedProxyModel::LayoutChanged() {
  for (QAbstractItemModel* key : merge_points_.keys()) {
    if (!old_merge_points_.contains(key)) continue;

    const int old_row = old_merge_points_[key].row();
    const int new_row = merge_points_[key].row();

    if (old_row != new_row) {
      reset();
      return;
    }
  }
}

bool MergedProxyModel::IsKnownModel(const QAbstractItemModel* model) const {
  if (model == this || model == sourceModel() ||
      merge_points_.contains(const_cast<QAbstractItemModel*>(model)))
    return true;
  return false;
}

QModelIndexList MergedProxyModel::mapFromSource(
    const QModelIndexList& source_indexes) const {
  QModelIndexList ret;
  for (const QModelIndex& index : source_indexes) {
    ret << mapFromSource(index);
  }
  return ret;
}

QModelIndexList MergedProxyModel::mapToSource(
    const QModelIndexList& proxy_indexes) const {
  QModelIndexList ret;
  for (const QModelIndex& index : proxy_indexes) {
    ret << mapToSource(index);
  }
  return ret;
}
