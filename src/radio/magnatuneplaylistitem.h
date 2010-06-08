#ifndef MAGNATUNEPLAYLISTITEM_H
#define MAGNATUNEPLAYLISTITEM_H

#include "library/libraryplaylistitem.h"

class MagnatunePlaylistItem : public LibraryPlaylistItem {
 public:
  MagnatunePlaylistItem(const QString& type);
  MagnatunePlaylistItem(const Song& song);

  bool InitFromQuery(const QSqlQuery &query);

  Options options() const;

  QUrl Url() const;
  SpecialLoadResult StartLoading();
};

#endif // MAGNATUNEPLAYLISTITEM_H
