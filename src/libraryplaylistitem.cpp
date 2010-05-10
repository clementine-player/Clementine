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

#include "libraryplaylistitem.h"

#include <QtDebug>
#include <QSettings>

LibraryPlaylistItem::LibraryPlaylistItem(const QString& type)
  : PlaylistItem(type)
{
}

LibraryPlaylistItem::LibraryPlaylistItem(const Song& song)
  : PlaylistItem("Library"),
    song_(song)
{
}


QUrl LibraryPlaylistItem::Url() const {
  return QUrl::fromLocalFile(song_.filename());
}

void LibraryPlaylistItem::Reload() {
  song_.InitFromFile(song_.filename(), song_.directory_id());
}

bool LibraryPlaylistItem::InitFromQuery(const QSqlQuery &query) {
  // Rows from the songs tables come first
  song_.InitFromQuery(query);

  return song_.is_valid();
}

QVariant LibraryPlaylistItem::DatabaseValue(DatabaseColumn column) const {
  switch (column) {
    case Column_LibraryId:    return song_.id();
    default:                  return PlaylistItem::DatabaseValue(column);
  }
}
