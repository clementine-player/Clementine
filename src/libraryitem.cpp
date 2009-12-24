#include "libraryitem.h"

LibraryItem::LibraryItem(Type _type, const QString& _key, LibraryItem* _parent)
  : type(_type),
    key(_key),
    lazy_loaded(_type == Type_Song || _type == Type_Root || _type == Type_Divider),
    parent(_parent)
{
  if (parent) {
    row = parent->children.count();
    parent->children << this;
  } else {
    row = 0;
  }
}

LibraryItem::~LibraryItem() {
  qDeleteAll(children);
}

void LibraryItem::Delete(int child_row) {
  delete children.takeAt(child_row);

  // Adjust row numbers of those below it :(
  for (int i=child_row ; i<children.count() ; ++i)
    children[i]->row --;
}

LibraryItem* LibraryItem::ChildByKey(const QString& key) const {
  foreach (LibraryItem* child, children) {
    if (child->key == key)
      return child;
  }
  return NULL;
}
