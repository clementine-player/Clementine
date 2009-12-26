#include "playlistitem.h"
#include "songplaylistitem.h"
#include "radioplaylistitem.h"

#include <QtDebug>

QString PlaylistItem::type_string() const {
  switch (type()) {
    case Type_Song: return "Song";
    case Type_Radio: return "Radio";
    default:
      qWarning() << "Invalid PlaylistItem type:" << type();
      return QString::null;
  }
}

PlaylistItem* PlaylistItem::NewFromType(const QString& type) {
  if (type == "Song")
    return new SongPlaylistItem;
  if (type == "Radio")
    return new RadioPlaylistItem;

  qWarning() << "Invalid PlaylistItem type:" << type;
  return NULL;
}
