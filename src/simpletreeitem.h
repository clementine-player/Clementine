#ifndef SIMPLETREEITEM_H
#define SIMPLETREEITEM_H

#include <QString>
#include <QList>

template <typename T>
class SimpleTreeItem {
 public:
  SimpleTreeItem(int _type, const QString& _key = QString::null, T* _parent = NULL);
  virtual ~SimpleTreeItem();

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
};

template <typename T>
SimpleTreeItem<T>::SimpleTreeItem(int _type, const QString& _key, T* _parent)
  : type(_type),
    key(_key),
    lazy_loaded(false),
    parent(_parent)
{
  if (parent) {
    row = parent->children.count();
    parent->children << static_cast<T*>(this);
  } else {
    row = 0;
  }
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
