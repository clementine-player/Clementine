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

#include <QFile>
#include <QIODevice>
#include <QUrl>
#include <QXmlStreamReader>

XSPFParser::XSPFParser(QObject* parent)
    : ParserBase(parent)
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

bool XSPFParser::ParseUntilElement(QXmlStreamReader* reader, const QString& name) const {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement:
        if (reader->name() == name) {
          return true;
        } else {
          IgnoreElement(reader);
        }
        break;
      default:
        break;
    }
  }
  return false;
}

void XSPFParser::IgnoreElement(QXmlStreamReader* reader) const {
  int level = 1;
  while (level != 0 && !reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement:
        ++level;
        break;
      case QXmlStreamReader::EndElement:
        --level;
        break;
      default:
        break;
    }
  }
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

}
