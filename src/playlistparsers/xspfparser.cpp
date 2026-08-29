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

#include "xspfparser.h"

#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QUrl>
#include <QXmlStreamReader>

#include "core/timeconstants.h"
#include "core/utilities.h"
#include "playlist/playlist.h"

XSPFParser::XSPFParser(LibraryBackendInterface* library, QObject* parent)
    : XMLParser(library, parent) {}

SongList XSPFParser::Load(QIODevice* device, const QString& playlist_path,
                          const QDir& dir) const {
  SongList ret;

  QXmlStreamReader reader(device);
  if (!Utilities::ParseUntilElement(&reader, "playlist") ||
      !Utilities::ParseUntilElement(&reader, "trackList")) {
    return ret;
  }

  while (!reader.atEnd() && Utilities::ParseUntilElement(&reader, "track")) {
    Song song = ParseTrack(&reader, dir);
    if (song.is_valid()) {
      ret << song;
    }
  }
  return ret;
}

Song XSPFParser::ParseTrack(QXmlStreamReader* reader, const QDir& dir) const {
  QString art, title, artist, album, location;
  qint64 nanosec = -1;
  int track_num = -1;

  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement: {
        QStringRef name = reader->name();
        if (name == "location") {
          location = reader->readElementText();
        } else if (name == "title") {
          title = reader->readElementText();
        } else if (name == "creator") {
          artist = reader->readElementText();
        } else if (name == "album") {
          album = reader->readElementText();
        } else if (name == "image") {
          art = reader->readElementText();
        } else if (name == "duration") {  // in milliseconds.
          const QString duration = reader->readElementText();
          bool ok = false;
          nanosec = duration.toInt(&ok) * kNsecPerMsec;
          if (!ok) {
            nanosec = -1;
          }
        } else if (name == "trackNum") {
          const QString track_num_str = reader->readElementText();
          bool ok = false;
          track_num = track_num_str.toInt(&ok);
          if (!ok || track_num < 1) {
            track_num = -1;
          }
        } else if (name == "info") {
          // TODO: Do something with extra info?
        }
        break;
      }
      case QXmlStreamReader::EndElement: {
        if (reader->name() == "track") {
          goto return_song;
        }
      }
      default:
        break;
    }
  }

return_song:
  Song song = LoadSong(location, 0, dir);

  // If the song is not in the library, fill metadata with what was in the
  // playlist.
  if (!song.is_library_song()) {
    song.set_title(title);
    song.set_artist(artist);
    song.set_album(album);
    song.set_art_manual(art);
    song.set_length_nanosec(nanosec);
    song.set_track(track_num);
  }

  return song;
}

void XSPFParser::Save(const SongList& songs, QIODevice* device, const QDir& dir,
                      Playlist::Path path_type) const {
  QFileInfo file;
  QXmlStreamWriter writer(device);
  writer.setAutoFormatting(true);
  writer.setAutoFormattingIndent(2);
  writer.writeStartDocument();
  StreamElement playlist("playlist", &writer);
  writer.writeAttribute("version", "1");
  writer.writeDefaultNamespace("http://xspf.org/ns/0/");

  QSettings s;
  s.beginGroup(Playlist::kSettingsGroup);
  bool writeMetadata = s.value(Playlist::kWriteMetadata, true).toBool();
  s.endGroup();

  StreamElement tracklist("trackList", &writer);
  for (const Song& song : songs) {
    QString filename_or_url = URLOrFilename(song.url(), dir, path_type);

    StreamElement track("track", &writer);
    writer.writeTextElement("location", filename_or_url);

    if (writeMetadata) {
      writer.writeTextElement("title", song.title());
      if (!song.artist().isEmpty()) {
        writer.writeTextElement("creator", song.artist());
      }
      if (!song.album().isEmpty()) {
        writer.writeTextElement("album", song.album());
      }
      if (song.length_nanosec() != -1) {
        writer.writeTextElement(
            "duration", QString::number(song.length_nanosec() / kNsecPerMsec));
      }
      if (song.track() > 0) {
        writer.writeTextElement("trackNum", QString::number(song.track()));
      }

      QString art = song.art_manual().isEmpty() ? song.art_automatic()
                                                : song.art_manual();
      // Ignore images that are in our resource bundle.
      if (!art.startsWith(":") && !art.isEmpty()) {
        QString art_filename;
        if (!art.contains("://")) {
          art_filename = art;
        } else if (QUrl(art).scheme() == "file") {
          art_filename = QUrl(art).toLocalFile();
        }

        if (!art_filename.isEmpty() && !(art_filename == "(embedded)")) {
          // Make this filename relative to the directory we're saving the
          // playlist.
          QUrl url = QUrl(art_filename);
          url.setScheme("file");  // Need to explicitly set this.
          art_filename = URLOrFilename(url, dir, path_type).toUtf8();
        } else {
          // Just use whatever URL was in the Song.
          art_filename = art;
        }

        writer.writeTextElement("image", art_filename);
      }
    }
  }
  writer.writeEndDocument();
}

bool XSPFParser::TryMagic(const QByteArray& data) const {
  return data.contains("<playlist") && data.contains("<trackList");
}
