#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include <QString>
#include <QList>

#include "song.h"
#include "simpletreeitem.h"

class LibraryItem : public SimpleTreeItem<LibraryItem> {
 public:
  enum Type {
    Type_Root,
    Type_Divider,
    Type_CompilationArtist,
    Type_CompilationAlbum,
    Type_Artist,
    Type_Album,
    Type_Song,
  };

  LibraryItem(SimpleTreeModel<LibraryItem>* model)
    : SimpleTreeItem<LibraryItem>(Type_Root, model) {}
  LibraryItem(Type type, const QString& key = QString::null, LibraryItem* parent = NULL)
    : SimpleTreeItem<LibraryItem>(type, key, parent) {}

  Song song;
};

#endif // LIBRARYITEM_H
