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
#include "libraryplaylistitem.h"

#include <QtDebug>

PlaylistItem* PlaylistItem::NewFromType(const QString& type) {
  if (type == "Library")
    return new LibraryPlaylistItem(type);
  if (type == "Stream" || type == "File")
    return new SongPlaylistItem(type);
  if (type == "Radio")
    return new RadioPlaylistItem(type);

  qWarning() << "Invalid PlaylistItem type:" << type;
  return NULL;
}

void PlaylistItem::BindToQuery(QSqlQuery* query) const {
  query->bindValue(":type", type());
  query->bindValue(":library_id", DatabaseValue(Column_LibraryId));
  query->bindValue(":url", DatabaseValue(Column_Url));
  query->bindValue(":title", DatabaseValue(Column_Title));
  query->bindValue(":artist", DatabaseValue(Column_Artist));
  query->bindValue(":album", DatabaseValue(Column_Album));
  query->bindValue(":length", DatabaseValue(Column_Length));
  query->bindValue(":radio_service", DatabaseValue(Column_RadioService));
}


