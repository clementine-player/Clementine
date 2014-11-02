/* This file is part of Clementine.
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef CORE_SIMPLETREEMODEL_H_
#define CORE_SIMPLETREEMODEL_H_

#include <QAbstractItemModel>

class QModelIndex;

template <typename T>
class SimpleTreeModel : public QAbstractItemModel {
 public:
  explicit SimpleTreeModel(T* root = 0, QObject* parent = nullptr);
  virtual ~SimpleTreeModel() {}

  // QAbstractItemModel
  int columnCount(const QModelIndex& parent) const;
  QModelIndex index(int row, int,
                    const QModelIndex& parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex& index) const;
  int rowCount(const QModelIndex& parent) const;
  bool hasChildren(const QModelIndex& parent) const;
  bool canFetchMore(const QModelIndex& parent) const;
  void fetchMore(const QModelIndex& parent);

  T* IndexToItem(const QModelIndex& index) const;
  QModelIndex ItemToIndex(T* item) const;

  // Called by items
  void BeginInsert(T* parent, int start, int end = -1);
  void EndInsert();
  void BeginDelete(T* parent, int start, int end = -1);
  void EndDelete();
  void EmitDataChanged(T* item);

 protected:
  virtual void LazyPopulate(T* item) = 0;

 protected:
  T* root_;
};

template <typename T>
SimpleTreeModel<T>::SimpleTreeModel(T* root, QObject* parent)
    : QAbstractItemModel(parent), root_(root) {}

template <typename T>
T* SimpleTreeModel<T>::IndexToItem(const QModelIndex& index) const {
  if (!index.isValid()) return root_;
  return reinterpret_cast<T*>(index.internalPointer());
}

template <typename T>
QModelIndex SimpleTreeModel<T>::ItemToIndex(T* item) const {
  if (!item || !item->parent) return QModelIndex();
  return createIndex(item->row, 0, item);
}

template <typename T>
int SimpleTreeModel<T>::columnCount(const QModelIndex&) const {
  return 1;
}

template <typename T>
QModelIndex SimpleTreeModel<T>::index(int row, int,
                                      const QModelIndex& parent) const {
  T* parent_item = IndexToItem(parent);
  if (!parent_item || row < 0 || parent_item->children.count() <= row)
    return QModelIndex();

  return ItemToIndex(parent_item->children[row]);
}

template <typename T>
QModelIndex SimpleTreeModel<T>::parent(const QModelIndex& index) const {
  return ItemToIndex(IndexToItem(index)->parent);
}

template <typename T>
int SimpleTreeModel<T>::rowCount(const QModelIndex& parent) const {
  T* item = IndexToItem(parent);
  return item->children.count();
}

template <typename T>
bool SimpleTreeModel<T>::hasChildren(const QModelIndex& parent) const {
  T* item = IndexToItem(parent);
  if (item->lazy_loaded)
    return !item->children.isEmpty();
  else
    return true;
}

template <typename T>
bool SimpleTreeModel<T>::canFetchMore(const QModelIndex& parent) const {
  T* item = IndexToItem(parent);
  return !item->lazy_loaded;
}

template <typename T>
void SimpleTreeModel<T>::fetchMore(const QModelIndex& parent) {
  T* item = IndexToItem(parent);
  if (!item->lazy_loaded) {
    LazyPopulate(item);
  }
}

template <typename T>
void SimpleTreeModel<T>::BeginInsert(T* parent, int start, int end) {
  if (end == -1) end = start;
  beginInsertRows(ItemToIndex(parent), start, end);
}

template <typename T>
void SimpleTreeModel<T>::EndInsert() {
  endInsertRows();
}

template <typename T>
void SimpleTreeModel<T>::BeginDelete(T* parent, int start, int end) {
  if (end == -1) end = start;
  beginRemoveRows(ItemToIndex(parent), start, end);
}

template <typename T>
void SimpleTreeModel<T>::EndDelete() {
  endRemoveRows();
}

template <typename T>
void SimpleTreeModel<T>::EmitDataChanged(T* item) {
  QModelIndex index(ItemToIndex(item));
  emit dataChanged(index, index);
}

#endif  // CORE_SIMPLETREEMODEL_H_
