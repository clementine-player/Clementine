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
#include "library/libraryplaylistitem.h"
#include "radio/magnatuneplaylistitem.h"
#include "radio/radioplaylistitem.h"

#include <QtDebug>

PlaylistItem::SpecialLoadResult::SpecialLoadResult(
    Type type, const QUrl& original_url, const QUrl& media_url)
      : type_(type), original_url_(original_url), media_url_(media_url)
{
}

PlaylistItem* PlaylistItem::NewFromType(const QString& type) {
  if (type == "Library")
    return new LibraryPlaylistItem(type);
  if (type == "Magnatune")
    return new MagnatunePlaylistItem(type);
  if (type == "Stream" || type == "File")
    return new SongPlaylistItem(type);
  if (type == "Radio")
    return new RadioPlaylistItem(type);

  qWarning() << "Invalid PlaylistItem type:" << type;
  return NULL;
}

void PlaylistItem::BindToQuery(QSqlQuery* query) const {
  query->bindValue(1, type());
  query->bindValue(2, DatabaseValue(Column_LibraryId));
  query->bindValue(3, DatabaseValue(Column_Url));
  query->bindValue(4, DatabaseValue(Column_Title));
  query->bindValue(5, DatabaseValue(Column_Artist));
  query->bindValue(6, DatabaseValue(Column_Album));
  query->bindValue(7, DatabaseValue(Column_Length));
  query->bindValue(8, DatabaseValue(Column_RadioService));
}

void PlaylistItem::SetTemporaryMetadata(const Song& metadata) {
  temp_metadata_ = metadata;
  temp_metadata_.set_filetype(Song::Type_Stream);
}

void PlaylistItem::ClearTemporaryMetadata() {
  temp_metadata_ = Song();
}


