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

#include "asxiniparser.h"
#include "core/logging.h"

#include <QTextStream>
#include <QtDebug>

AsxIniParser::AsxIniParser(LibraryBackendInterface* library, QObject* parent)
    : ParserBase(library, parent) {}

bool AsxIniParser::TryMagic(const QByteArray& data) const {
  return data.toLower().contains("[reference]");
}

SongList AsxIniParser::Load(QIODevice* device, const QString& playlist_path,
                            const QDir& dir) const {
  SongList ret;

  while (!device->atEnd()) {
    QString line = QString::fromUtf8(device->readLine()).trimmed();
    int equals = line.indexOf('=');
    QString key = line.left(equals).toLower();
    QString value = line.mid(equals + 1);

    if (key.startsWith("ref")) {
      Song song = LoadSong(value, 0, dir);
      if (song.is_valid()) {
        ret << song;
      }
    }
  }

  return ret;
}

void AsxIniParser::Save(const SongList& songs, QIODevice* device,
                        const QDir& dir, Playlist::Path path_type) const {
  QTextStream s(device);
  s << "[Reference]" << endl;

  int n = 1;
  for (const Song& song : songs) {
    s << "Ref" << n << "=" << URLOrFilename(song.url(), dir, path_type) << endl;
    ++n;
  }
}
