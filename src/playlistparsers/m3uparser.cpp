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

#include "m3uparser.h"

#include <QBuffer>
#include <QtDebug>

M3UParser::M3UParser(LibraryBackendInterface* library, QObject* parent)
    : ParserBase(library, parent)
{
}

SongList M3UParser::Load(QIODevice* device, const QString& playlist_path, const QDir& dir) const {
  SongList ret;

  M3UType type = STANDARD;
  Metadata current_metadata;

  QString data = QString::fromUtf8(device->readAll());
  data.replace('\r', '\n');
  data.replace("\n\n", "\n");
  QByteArray bytes = data.toUtf8();
  QBuffer buffer(&bytes);
  buffer.open(QIODevice::ReadOnly);

  QString line = QString::fromUtf8(buffer.readLine()).trimmed();
  qDebug() << line;
  if (line.startsWith("#EXTM3U")) {
    // This is in extended M3U format.
    type = EXTENDED;
    line = QString::fromUtf8(buffer.readLine()).trimmed();
  }

  forever {
    qDebug() << line;
    if (line.startsWith('#')) {
      // Extended info or comment.
      if (type == EXTENDED && line.startsWith("#EXT")) {
        if (!ParseMetadata(line, &current_metadata)) {
          qWarning() << "Failed to parse metadata: " << line;
          continue;
        }
      }
    } else {
      Song song;

      // Track location.
      if (!ParseTrackLocation(line, dir, &song)) {
        qWarning() << "Failed to parse location: " << line;
      } else {
        // Load the song from the library if it's there.
        Song library_song = LoadLibrarySong(song.filename());
        if (library_song.is_valid()) {
          ret << library_song;
        } else {
          song.Init(current_metadata.title,
                    current_metadata.artist,
                    QString(),  // Unknown album.
                    current_metadata.length);
          song.InitFromFile(song.filename(), -1);
          ret << song;
        }

        current_metadata.artist.clear();
        current_metadata.title.clear();
        current_metadata.length = -1;
      }
    }
    if (buffer.atEnd()) {
      break;
    }
    line = QString::fromUtf8(buffer.readLine()).trimmed();
  }

  return ret;
}

bool M3UParser::ParseMetadata(const QString& line, M3UParser::Metadata* metadata) const {
  // Extended info, eg.
  // #EXTINF:123,Sample Artist - Sample title
  QString info = line.section(':', 1);
  QString l = info.section(',', 0, 0);
  bool ok = false;
  int length = l.toInt(&ok);
  if (!ok) {
    return false;
  }
  metadata->length = length * 1e9;

  QString track_info = info.section(',', 1);
  QStringList list = track_info.split('-');
  if (list.size() <= 1) {
    metadata->title = track_info;
    return true;
  }
  metadata->artist = list[0].trimmed();
  metadata->title = list[1].trimmed();
  return true;
}

void M3UParser::Save(const SongList &songs, QIODevice *device, const QDir &dir) const {
  device->write("#EXTM3U\n");
  foreach (const Song& song, songs) {
    if (song.filename().isEmpty()) {
      continue;
    }
    QString meta = QString("#EXTINF:%1,%2 - %3\n")
                   .arg(song.length_nanosec() / 1e9)
                   .arg(song.artist()).arg(song.title());
    device->write(meta.toUtf8());
    device->write(MakeRelativeTo(song.filename(), dir).toUtf8());
    device->write("\n");
  }
}

bool M3UParser::TryMagic(const QByteArray &data) const {
  return data.contains("#EXTM3U") || data.contains("#EXTINF");
}
