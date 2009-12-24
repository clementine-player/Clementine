#include "playlistitem.h"
#include "songplaylistitem.h"

#include <QtDebug>

QString PlaylistItem::type_string() const {
  switch (type()) {
    case Type_Song: return "Song";
    default:
      qWarning() << "Invalid PlaylistItem type:" << type();
      return QString::null;
  }
}

PlaylistItem* PlaylistItem::NewFromType(const QString& type) {
  if (type == "Song")
    return new SongPlaylistItem;

  qWarning() << "Invalid PlaylistItem type:" << type;
  return NULL;
}
