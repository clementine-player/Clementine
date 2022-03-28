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
#include <QRegExp>
#include <QTextCodec>
#include <QtDebug>

#include "core/logging.h"
#include "core/timeconstants.h"
#include "playlist/playlist.h"

M3UParser::M3UParser(LibraryBackendInterface* library, QObject* parent)
    : ParserBase(library, parent) {}

SongList M3UParser::Load(QIODevice* device, const QString& playlist_path,
                         const QDir& dir) const {
  SongList ret;

  M3UType type = STANDARD;
  Metadata current_metadata;

  // Unicode auto-detection is enabled in QTextStream by default.
  QTextStream playlist_stream(device);
  QString data = playlist_stream.readAll();
  qLog(Debug) << "Detected codec" << playlist_stream.codec()->name();

  // iTune playlists use \r newlines. These aren't handled by the Qt readLine
  // methods.
  QStringList lines = data.split(QRegExp("\n|\r|\r\n"));

  for (int i = 0; i < lines.count(); i++) {
    QString line = lines[i].trimmed();
    if (i == 0 && line.startsWith("#EXTM3U")) {
      // This is in extended M3U format.
      type = EXTENDED;
    } else if (line.startsWith('#')) {
      // Extended info or comment.
      if (type == EXTENDED && line.startsWith("#EXT")) {
        if (!ParseMetadata(line, &current_metadata)) {
          qLog(Warning) << "Failed to parse metadata: " << line;
        }
      }
    } else if (!line.isEmpty()) {
      Song song = LoadSong(line, 0, dir);

      // Override metadata from playlist if the song is not in the library.
      if (!song.is_library_song()) {
        if (!current_metadata.title.isEmpty()) {
          song.set_title(current_metadata.title);
        }
        if (!current_metadata.artist.isEmpty()) {
          song.set_artist(current_metadata.artist);
        }
        if (current_metadata.length > 0) {
          song.set_length_nanosec(current_metadata.length);
        }
      }

      ret << song;

      current_metadata = Metadata();
    }
  }

  return ret;
}

bool M3UParser::ParseMetadata(const QString& line,
                              M3UParser::Metadata* metadata) const {
  // Extended info, eg.
  // #EXTINF:123,Sample Artist - Sample title
  QString info = line.section(':', 1);
  QString l = info.section(',', 0, 0);
  bool ok = false;
  int length = l.toInt(&ok);
  if (!ok) {
    return false;
  }
  metadata->length = length * kNsecPerSec;

  QString track_info = info.section(',', 1);
  QStringList list = track_info.split(" - ");
  if (list.size() <= 1) {
    metadata->title = track_info;
    return true;
  }
  metadata->artist = list[0].trimmed();
  metadata->title = list[1].trimmed();
  return true;
}

void M3UParser::Save(const SongList& songs, QIODevice* device, const QDir& dir,
                     Playlist::Path path_type) const {
  device->write("#EXTM3U\n");

  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  bool writeMetadata = s.value(Playlist::kWriteMetadata, true).toBool();
  s.endGroup();

  for (const Song& song : songs) {
    if (song.url().isEmpty()) {
      continue;
    }
    if (writeMetadata) {
      QString meta = QString("#EXTINF:%1,%2 - %3\n")
                         .arg(song.length_nanosec() / kNsecPerSec)
                         .arg(song.artist())
                         .arg(song.title());
      device->write(meta.toUtf8());
    }
    device->write(URLOrFilename(song.url(), dir, path_type).toUtf8());
    device->write("\n");
  }
}

bool M3UParser::TryMagic(const QByteArray& data) const {
  return data.contains("#EXTM3U") || data.contains("#EXTINF");
}
