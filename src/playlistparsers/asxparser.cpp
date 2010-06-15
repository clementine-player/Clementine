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

#include "asxparser.h"

#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QUrl>
#include <QXmlStreamReader>

ASXParser::ASXParser(QObject* parent)
    : XMLParser(parent)
{
}

SongList ASXParser::Load(QIODevice *device, const QDir&) const {
  SongList ret;

  QXmlStreamReader reader(device);
  if (!ParseUntilElement(&reader, "asx")) {
    return ret;
  }

  while (!reader.atEnd() && ParseUntilElement(&reader, "entry")) {
    Song song = ParseTrack(&reader);
    if (song.is_valid()) {
      ret << song;
    }
  }
  return ret;
}


Song ASXParser::ParseTrack(QXmlStreamReader* reader) const {
  Song song;
  QString title, artist, album;
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement: {
        QStringRef name = reader->name();
        if (name == "ref") {
          QUrl url(reader->attributes().value("href").toString());
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
        } else if (name == "author") {
          artist = reader->readElementText();
        }
        break;
      }
      case QXmlStreamReader::EndElement: {
        if (reader->name() == "entry") {
          song.Init(title, artist, album, -1);
          return song;
        }
      }
      default:
        break;
    }
  }
  // At least make an effort if we never find a </entry>.
  song.Init(title, artist, album, -1);
  return song;
}

void ASXParser::Save(const SongList &songs, QIODevice *device, const QDir &dir) const {
  QXmlStreamWriter writer(device);
  writer.setAutoFormatting(true);
  writer.writeStartDocument();
  {
    StreamElement asx("asx", &writer);
    writer.writeAttribute("version", "3.0");
    foreach (const Song& song, songs) {
      StreamElement entry("entry", &writer);
      writer.writeTextElement("title", song.title());
      {
        StreamElement ref("ref", &writer);
        writer.writeAttribute("href", MakeRelativeTo(song.filename(), dir));
      }
      if (!song.artist().isEmpty()) {
        writer.writeTextElement("author", song.artist());
      }
    }
  }
  writer.writeEndDocument();
}

bool ASXParser::TryMagic(const QByteArray &data) const {
  return data.toLower().contains("<asx");
}
