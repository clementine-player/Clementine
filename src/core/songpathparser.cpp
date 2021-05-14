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

#include "songpathparser.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include "core/logging.h"
#include "song.h"

const char* SongPathParser::kSongMetadataSettingsGroup = "SongMetadata";
const char* SongPathParser::kGuessMetadataSetting = "guess_metadata";
const bool SongPathParser::kGuessMetadataSettingDefault = true;

SongPathParser::SongPathParser() : guess_metadata_(true) { ReloadSettings(); }

void SongPathParser::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSongMetadataSettingsGroup);
  guess_metadata_ =
      s.value(kGuessMetadataSetting, kGuessMetadataSettingDefault).toBool();
}

// Helpers for GuessArtistAndTitle()
static QString WithoutExtension(const QString& s) {
  if (s.isEmpty()) return s;
  const int i = s.lastIndexOf('.');
  if (i < 0) return s;
  return s.left(i);
}

static QString ReplaceUnderscoresWithSpaces(const QString& s) {
  QString ret(s);
  ret.replace('_', ' ');
  return ret;
}

void SongPathParser::GuessArtistAndTitle(Song* song) {
  qLog(Debug) << "Guess artist and title";
  QString artist = song->artist();
  QString title = song->title();
  const QString bn = song->basefilename();
  if (!artist.isEmpty() || !title.isEmpty()) return;
  if (bn.isEmpty()) return;

  QRegExp rx("^(.*)[\\s_]\\-[\\s_](.*)\\.\\w*$");
  if (rx.indexIn(bn) >= 0) {
    artist = rx.cap(1);
    title = rx.cap(2);
  } else {
    title = WithoutExtension(bn);
  }

  artist = ReplaceUnderscoresWithSpaces(artist);
  title = ReplaceUnderscoresWithSpaces(title);
  artist = artist.trimmed();
  title = title.trimmed();
  if (!artist.isEmpty()) {
    song->set_artist(artist);
  }
  if (!title.isEmpty()) {
    song->set_title(title);
  }
}

void SongPathParser::GuessAlbum(const QString& path, Song* song) {
  qLog(Debug) << "Guess album";
  QFileInfo info(path);
  QString album = song->album();
  if (!album.isEmpty()) return;
  const QString str_dir = info.absoluteDir().absolutePath();
  if (str_dir.isEmpty()) return;
  const QFileInfo dir(str_dir);
  const QString dir_bn = dir.baseName();
  if (dir_bn.isEmpty()) return;
  album = ReplaceUnderscoresWithSpaces(dir_bn);
  album = album.trimmed();
  if (album.isEmpty()) return;
  const QString al = album.toLower();
  if (al == "various" || al == "downloads" || al == "music") return;
  song->set_album(album);
}

void SongPathParser::GuessMissingFields(Song* song, QString path) {
  if (guess_metadata_) {
    GuessArtistAndTitle(song);
    GuessAlbum(path, song);
  }
}
