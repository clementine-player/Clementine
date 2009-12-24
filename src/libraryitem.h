#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include <QString>
#include <QList>

#include "song.h"

struct LibraryItem {
  enum Type {
    Type_Root,
    Type_Divider,
    Type_CompilationArtist,
    Type_CompilationAlbum,
    Type_Artist,
    Type_Album,
    Type_Song,
  };

  LibraryItem(Type _type, const QString& _key = QString::null, LibraryItem* _parent = NULL);
  ~LibraryItem();

  void Delete(int child_row);
  LibraryItem* ChildByKey(const QString& key) const;

  QString DisplayText() const { return display_text.isNull() ? key : display_text; }
  QString SortText() const { return sort_text.isNull() ? key : sort_text; }

  Type type;
  QString key;
  QString sort_text;
  QString display_text;
  int row;
  bool lazy_loaded;
  Song song;

  LibraryItem* parent;
  QList<LibraryItem*> children;
};

#endif // LIBRARYITEM_H
