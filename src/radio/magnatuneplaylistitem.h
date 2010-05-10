#ifndef MAGNATUNEPLAYLISTITEM_H
#define MAGNATUNEPLAYLISTITEM_H

#include "library/libraryplaylistitem.h"

class MagnatunePlaylistItem : public LibraryPlaylistItem {
 public:
  MagnatunePlaylistItem(const QString& type);
  MagnatunePlaylistItem(const Song& song);

  bool InitFromQuery(const QSqlQuery &query);
};

#endif // MAGNATUNEPLAYLISTITEM_H
