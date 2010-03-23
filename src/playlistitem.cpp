/* This file is part of Clementine.

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
