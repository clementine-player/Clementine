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
  SongPlaylistItem();
  SongPlaylistItem(const Song& song);

  Type type() const { return Type_Song; }

  void Save(SettingsProvider& settings) const;
  void Restore(const SettingsProvider& settings);
  void Reload();

  Song Metadata() const { return song_; }

  QUrl Url() const;

 private:
  void SaveFile(SettingsProvider& settings) const;
  void SaveStream(SettingsProvider& settings) const;

  void RestoreFile(const SettingsProvider& settings);
  void RestoreStream(const SettingsProvider& settings);
  Song song_;
};

#endif // SONGPLAYLISTITEM_H
