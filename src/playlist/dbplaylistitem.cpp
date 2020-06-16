/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2020, Jim Broadus <jbroadus@gmail.com>

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

#include "dbplaylistitem.h"
#include "core/tagreaderclient.h"

DbPlaylistItem::DbPlaylistItem(const QString& type)
    : PlaylistItem(type) {}

DbPlaylistItem::DbPlaylistItem(const QString& type, const Song& song)
    : PlaylistItem(type), song_(song) {}

QUrl DbPlaylistItem::Url() const { return song_.url(); }

QVariant DbPlaylistItem::DatabaseValue(DatabaseColumn column) const {
  switch (column) {
    case Column_LibraryId:
      return song_.id();
    default:
      return PlaylistItem::DatabaseValue(column);
  }
}

Song DbPlaylistItem::Metadata() const {
  if (HasTemporaryMetadata()) return temp_metadata_;
  return song_;
}
