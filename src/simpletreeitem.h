#ifndef SIMPLETREEITEM_H
#define SIMPLETREEITEM_H

#include "simpletreemodel.h"

#include <QString>
#include <QList>

template <typename T>
class SimpleTreeItem {
 public:
  SimpleTreeItem(int _type, SimpleTreeModel<T>* _model); // For the root item
  SimpleTreeItem(int _type, const QString& _key = QString::null, T* _parent = NULL);
  virtual ~SimpleTreeItem();

  void InsertNotify(T* _parent);
  void DeleteNotify(int child_row);
  void ClearNotify();

  void Delete(int child_row);
  T* ChildByKey(const QString& key) const;

  QString DisplayText() const { return display_text.isNull() ? key : display_text; }
  QString SortText() const { return sort_text.isNull() ? key : sort_text; }

  int type;
  QString key;
  QString sort_text;
  QString display_text;

  int row;
  bool lazy_loaded;

  T* parent;
  QList<T*> children;

  SimpleTreeModel<T>* model;
};

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem(int _type, SimpleTreeModel<T>* _model)
  : type(_type),
    row(0),
    lazy_loaded(true),
    parent(NULL),
    model(_model)
{
}

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem(int _type, const QString& _key, T* _parent)
  : type(_type),
    key(_key),
    lazy_loaded(false),
    parent(_parent),
    model(_parent ? _parent->model : NULL)
{
  if (parent) {
    row = parent->children.count();
    parent->children << static_cast<T*>(this);
  }
}


template <typename T>
void SimpleTreeItem<T>::InsertNotify(T* _parent) {
  parent = _parent;
  model = parent->model;
  row = parent->children.count();

  model->BeginInsert(parent, row);
  parent->children << static_cast<T*>(this);
  model->EndInsert();
}

template <typename T>
void SimpleTreeItem<T>::DeleteNotify(int child_row) {
  model->BeginDelete(static_cast<T*>(this), child_row);
  delete children.takeAt(child_row);

  // Adjust row numbers of those below it :(
  for (int i=child_row ; i<children.count() ; ++i)
    children[i]->row --;
  model->EndDelete();
}

template <typename T>
void SimpleTreeItem<T>::ClearNotify() {
  model->BeginDelete(static_cast<T*>(this), 0, children.count()-1);

  qDeleteAll(children);
  children.clear();

  model->EndDelete();
}

template <typename T>
SimpleTreeItem<T>::~SimpleTreeItem() {
  qDeleteAll(children);
}

template <typename T>
void SimpleTreeItem<T>::Delete(int child_row) {
  delete children.takeAt(child_row);

  // Adjust row numbers of those below it :(
  for (int i=child_row ; i<children.count() ; ++i)
    children[i]->row --;
}

template <typename T>
T* SimpleTreeItem<T>::ChildByKey(const QString& key) const {
  foreach (T* child, children) {
    if (child->key == key)
      return child;
  }
  return NULL;
}

#endif // SIMPLETREEITEM_H
