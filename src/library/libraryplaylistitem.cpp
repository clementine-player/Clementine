/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "libraryplaylistitem.h"
#include "core/tagreaderclient.h"

#include <QSettings>

LibraryPlaylistItem::LibraryPlaylistItem(const QString& type)
    : PlaylistItem(type) {}

LibraryPlaylistItem::LibraryPlaylistItem(const Song& song)
    : PlaylistItem("Library"), song_(song) {}

QUrl LibraryPlaylistItem::Url() const { return song_.url(); }

void LibraryPlaylistItem::Reload() {
  TagReaderClient::Instance()->ReadFileBlocking(song_.url().toLocalFile(),
                                                &song_);
}

bool LibraryPlaylistItem::InitFromQuery(const SqlRow& query) {
  // Rows from the songs tables come first
  song_.InitFromQuery(query, true);

  return song_.is_valid();
}

QVariant LibraryPlaylistItem::DatabaseValue(DatabaseColumn column) const {
  switch (column) {
    case Column_LibraryId:
      return song_.id();
    default:
      return PlaylistItem::DatabaseValue(column);
  }
}

Song LibraryPlaylistItem::Metadata() const {
  if (HasTemporaryMetadata()) return temp_metadata_;
  return song_;
}
