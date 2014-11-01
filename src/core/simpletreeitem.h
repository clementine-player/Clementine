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

#ifndef CORE_SIMPLETREEITEM_H_
#define CORE_SIMPLETREEITEM_H_

#include "simpletreemodel.h"

#include <QString>
#include <QList>

template <typename T>
class SimpleTreeItem {
 public:
  SimpleTreeItem(int _type, SimpleTreeModel<T>* _model);  // For the root item
  SimpleTreeItem(int _type, const QString& _key, T* _parent = nullptr);
  SimpleTreeItem(int _type, T* _parent = nullptr);
  virtual ~SimpleTreeItem();

  void InsertNotify(T* _parent);
  void DeleteNotify(int child_row);
  void ClearNotify();
  void ChangedNotify();

  void Delete(int child_row);
  T* ChildByKey(const QString& key) const;

  QString DisplayText() const {
    return display_text.isNull() ? key : display_text;
  }
  QString SortText() const { return sort_text.isNull() ? key : sort_text; }

  int type;
  QString key;
  QString sort_text;
  QString display_text;

  int row;
  bool lazy_loaded;

  T* parent;
  QList<T*> children;
  QAbstractItemModel* child_model;

  SimpleTreeModel<T>* model;
};

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem(int _type, SimpleTreeModel<T>* _model)
    : type(_type),
      row(0),
      lazy_loaded(true),
      parent(nullptr),
      child_model(nullptr),
      model(_model) {}

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem(int _type, const QString& _key, T* _parent)
    : type(_type),
      key(_key),
      lazy_loaded(false),
      parent(_parent),
      child_model(nullptr),
      model(_parent ? _parent->model : nullptr) {
  if (parent) {
    row = parent->children.count();
    parent->children << static_cast<T*>(this);
  }
}

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem(int _type, T* _parent)
    : type(_type),
      lazy_loaded(false),
      parent(_parent),
      child_model(nullptr),
      model(_parent ? _parent->model : nullptr) {
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
  for (int i = child_row; i < children.count(); ++i) children[i]->row--;
  model->EndDelete();
}

template <typename T>
void SimpleTreeItem<T>::ClearNotify() {
  if (children.count()) {
    model->BeginDelete(static_cast<T*>(this), 0, children.count() - 1);

    qDeleteAll(children);
    children.clear();

    model->EndDelete();
  }
}

template <typename T>
void SimpleTreeItem<T>::ChangedNotify() {
  model->EmitDataChanged(static_cast<T*>(this));
}

template <typename T>
SimpleTreeItem<T>::~SimpleTreeItem() {
  qDeleteAll(children);
}

template <typename T>
void SimpleTreeItem<T>::Delete(int child_row) {
  delete children.takeAt(child_row);

  // Adjust row numbers of those below it :(
  for (int i = child_row; i < children.count(); ++i) children[i]->row--;
}

template <typename T>
T* SimpleTreeItem<T>::ChildByKey(const QString& key) const {
  for (T* child : children) {
    if (child->key == key) return child;
  }
  return nullptr;
}

#endif  // CORE_SIMPLETREEITEM_H_
