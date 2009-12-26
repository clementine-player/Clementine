#ifndef SIMPLETREEMODEL_H
#define SIMPLETREEMODEL_H

#include <QAbstractItemModel>

class QModelIndex;

template <typename T>
class SimpleTreeModel : public QAbstractItemModel {
 public:
  SimpleTreeModel(T* root = 0, QObject* parent = 0);
  virtual ~SimpleTreeModel() {}

  // QAbstractItemModel
  int columnCount(const QModelIndex& parent) const;
  QModelIndex index(int row, int, const QModelIndex& parent) const;
  QModelIndex parent(const QModelIndex& index) const;
  int rowCount(const QModelIndex& parent) const;
  bool hasChildren(const QModelIndex& parent) const;

 protected:
  T* IndexToItem(const QModelIndex& index) const;
  QModelIndex ItemToIndex(T* item) const;

  virtual void LazyPopulate(T* item) = 0;

 protected:
  T* root_;
};


template <typename T>
SimpleTreeModel<T>::SimpleTreeModel(T* root, QObject* parent)
  : QAbstractItemModel(parent),
    root_(root)
{
}

template <typename T>
T* SimpleTreeModel<T>::IndexToItem(const QModelIndex& index) const {
  if (!index.isValid())
    return root_;
  return reinterpret_cast<T*>(index.internalPointer());
}

template <typename T>
QModelIndex SimpleTreeModel<T>::ItemToIndex(T* item) const {
  if (!item || !item->parent)
    return QModelIndex();
  return createIndex(item->row, 0, item);
}

template <typename T>
int SimpleTreeModel<T>::columnCount(const QModelIndex &) const {
  return 1;
}

template <typename T>
QModelIndex SimpleTreeModel<T>::index(int row, int, const QModelIndex& parent) const {
  T* parent_item = IndexToItem(parent);
  if (!parent_item || parent_item->children.count() <= row)
    return QModelIndex();

  return ItemToIndex(parent_item->children[row]);
}

template <typename T>
QModelIndex SimpleTreeModel<T>::parent(const QModelIndex& index) const {
  return ItemToIndex(IndexToItem(index)->parent);
}

template <typename T>
int SimpleTreeModel<T>::rowCount(const QModelIndex & parent) const {
  T* item = IndexToItem(parent);
  const_cast<SimpleTreeModel<T>*>(this)->LazyPopulate(item); // Ahem

  return item->children.count();
}

template <typename T>
bool SimpleTreeModel<T>::hasChildren(const QModelIndex &parent) const {
  T* item = IndexToItem(parent);
  if (item->lazy_loaded)
    return !item->children.isEmpty();
  else
    return true;
}

#endif // SIMPLETREEMODEL_H
