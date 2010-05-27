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

#include "xspfparser.h"

#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QUrl>
#include <QXmlStreamReader>

XSPFParser::XSPFParser(QObject* parent)
    : XMLParser(parent)
{
}

SongList XSPFParser::Load(QIODevice *device, const QDir&) const {
  SongList ret;

  QXmlStreamReader reader(device);
  if (!ParseUntilElement(&reader, "playlist") ||
      !ParseUntilElement(&reader, "trackList")) {
    return ret;
  }

  while (!reader.atEnd() && ParseUntilElement(&reader, "track")) {
    Song song = ParseTrack(&reader);
    if (song.is_valid()) {
      ret << song;
    }
  }
  return ret;
}

Song XSPFParser::ParseTrack(QXmlStreamReader* reader) const {
  Song song;
  QString title, artist, album;
  int length = -1;
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement: {
        QStringRef name = reader->name();
        if (name == "location") {
          QUrl url(reader->readElementText());
          if (url.scheme() == "file") {
            QString filename = url.toLocalFile();
            if (!QFile::exists(filename)) {
              return Song();
            }
            song.InitFromFile(filename, -1);
            return song;
          } else {
            song.set_filename(url.toString());
            song.set_filetype(Song::Type_Stream);
          }
        } else if (name == "title") {
          title = reader->readElementText();
        } else if (name == "creator") {
          artist = reader->readElementText();
        } else if (name == "album") {
          album = reader->readElementText();
        } else if (name == "duration") {  // in milliseconds.
          const QString& duration = reader->readElementText();
          bool ok = false;
          length = duration.toInt(&ok) / 1000;
          if (!ok) {
            length = -1;
          }
        } else if (name == "image") {
          // TODO: Fetch album covers.
        } else if (name == "info") {
          // TODO: Do something with extra info?
        }
        break;
      }
      case QXmlStreamReader::EndElement: {
        if (reader->name() == "track") {
          song.Init(title, artist, album, length);
          return song;
        }
      }
      default:
        break;
    }
  }
  // At least make an effort if we never find a </track>.
  song.Init(title, artist, album, length);
  return song;
}

void XSPFParser::Save(const SongList &songs, QIODevice *device, const QDir &dir) const {
  QDomDocument doc;
  QDomElement root = doc.createElement("playlist");
  doc.appendChild(root);
  QDomElement track_list = doc.createElement("trackList");
  root.appendChild(track_list);
  foreach (const Song& song, songs) {
    QString url = MakeRelativeTo(song.filename(), dir);
    if (url.isEmpty()) {
      continue;  // Skip empty items like Last.fm streams.
    }
    QDomElement track = doc.createElement("track");
    track_list.appendChild(track);
    MaybeAppendElementWithText("location", url, &doc, &track);
    MaybeAppendElementWithText("creator", song.artist(), &doc, &track);
    MaybeAppendElementWithText("album", song.album(), &doc, &track);
    MaybeAppendElementWithText("title", song.title(), &doc, &track);
    if (song.length() != -1) {
      MaybeAppendElementWithText("duration", QString::number(song.length() * 1000), &doc, &track);
    }
    QString art = song.art_manual().isEmpty() ? song.art_automatic() : song.art_manual();
    // Ignore images that are in our resource bundle.
    if (!art.startsWith(":") && !art.isEmpty()) {
      // Convert local files to URLs.
      if (!art.contains(QRegExp("^\\w+://"))) {
        art = QUrl::fromLocalFile(MakeRelativeTo(art, dir)).toString();
      }
      MaybeAppendElementWithText("image", art, &doc, &track);
    }
  }

  device->write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  device->write(doc.toByteArray(2));
}
