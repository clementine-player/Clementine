/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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
#include "syncsong.h"

#include <QUrl>

#include "core/song.h"

namespace SyncSong {

UrlHandler::Downloadability Downloadability(const Song& song,
                                            const UrlHandler* handler) {
  // The same test InternetBrowser uses to call a node a stream rather than
  // a track.
  if (song.is_stream() ||
      (song.url().scheme() != "file" && song.length_nanosec() <= 0)) {
    return UrlHandler::Downloadability::NotDownloadable;
  }
  if (handler) return handler->downloadability(song);
  return UrlHandler::Downloadability::Downloadable;
}

QString ItemKey(const Song& song) {
  if (song.is_library_song()) return "library/" + QString::number(song.id());
  if (song.url().isEmpty()) return QString();
  return "url/" + QString::fromLatin1(QUrl::toPercentEncoding(
                      song.url().toString(QUrl::FullyEncoded)));
}

QString ItemVersion(const Song& song, const QString& preset) {
  QString version =
      QString::number(song.mtime()) + '-' + QString::number(song.filesize());
  if (!preset.isEmpty()) version += '-' + preset;
  return version;
}

}  // namespace SyncSong
