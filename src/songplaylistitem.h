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

#ifndef SONGPLAYLISTITEM_H
#define SONGPLAYLISTITEM_H

#include "playlistitem.h"
#include "song.h"

class SongPlaylistItem : public PlaylistItem {
 public:
  SongPlaylistItem(const QString& type);
  SongPlaylistItem(const Song& song);

  bool InitFromQuery(const QSqlQuery &query);
  void Reload();

  Song Metadata() const { return song_; }

  QUrl Url() const;

 protected:
  QVariant DatabaseValue(DatabaseColumn) const;

 private:
  Song song_;
};

#endif // SONGPLAYLISTITEM_H
