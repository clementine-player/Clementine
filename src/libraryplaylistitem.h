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

#ifndef LIBRARYPLAYLISTITEM_H
#define LIBRARYPLAYLISTITEM_H

#include "playlistitem.h"
#include "song.h"

class LibraryPlaylistItem : public PlaylistItem {
 public:
  LibraryPlaylistItem(const QString& type);
  LibraryPlaylistItem(const Song& song);

  void InitFromQuery(const QSqlQuery &query);
  void BindToQuery(QSqlQuery *query) const;
  void Reload();

  Song Metadata() const { return song_; }

  QUrl Url() const;

 protected:
  QVariant DatabaseValue(DatabaseColumn column) const;

 private:
  Song song_;
};

#endif // LIBRARYPLAYLISTITEM_H
