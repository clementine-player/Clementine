/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#ifndef CORE_SONGPATHPARSER_H_
#define CORE_SONGPATHPARSER_H_

#include <QString>

class Song;

class SongPathParser {
 public:
  SongPathParser();

  static const char* kSongMetadataSettingsGroup;
  static const char* kGuessMetadataSetting;
  static const bool kGuessMetadataSettingDefault;

  void GuessMissingFields(Song* song, QString path);
  void ReloadSettings();

 private:
  void GuessArtistAndTitle(Song* song);
  void GuessAlbum(const QString& path, Song* song);

  bool guess_metadata_;
};

#endif  // CORE_SONGPATHPARSER_H_
