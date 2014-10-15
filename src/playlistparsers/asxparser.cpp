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

#include "asxparser.h"
#include "core/utilities.h"

#include <QBuffer>
#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QUrl>
#include <QXmlStreamReader>
#include <QtDebug>

ASXParser::ASXParser(LibraryBackendInterface* library, QObject* parent)
    : XMLParser(library, parent) {}

SongList ASXParser::Load(QIODevice* device, const QString& playlist_path,
                         const QDir& dir) const {
  // We have to load everything first so we can munge the "XML".
  QByteArray data = device->readAll();

  // (thanks Amarok...)
  // ASX looks a lot like xml, but doesn't require tags to be case sensitive,
  // meaning we have to accept things like: <Abstract>...</abstract>
  // We use a dirty way to achieve this: we make all tags lower case
  QRegExp ex("(<[/]?[^>]*[A-Z]+[^>]*>)");
  ex.setCaseSensitivity(Qt::CaseInsensitive);
  int index = 0;
  while ((index = ex.indexIn(data, index)) != -1) {
    data.replace(ex.cap(1).toLocal8Bit(), ex.cap(1).toLower().toLocal8Bit());
    index += ex.matchedLength();
  }

  // Some playlists have unescaped & characters in URLs :(
  ex.setPattern("(href\\s*=\\s*\")([^\"]+)\"");
  index = 0;
  while ((index = ex.indexIn(data, index)) != -1) {
    QString url = ex.cap(2);
    url.replace(QRegExp("&(?!amp;|quot;|apos;|lt;|gt;)"), "&amp;");

    QByteArray replacement = QString(ex.cap(1) + url + "\"").toLocal8Bit();
    data.replace(ex.cap(0).toLocal8Bit(), replacement);
    index += replacement.length();
  }

  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);

  SongList ret;

  QXmlStreamReader reader(&buffer);
  if (!Utilities::ParseUntilElement(&reader, "asx")) {
    return ret;
  }

  while (!reader.atEnd() && Utilities::ParseUntilElement(&reader, "entry")) {
    Song song = ParseTrack(&reader, dir);
    if (song.is_valid()) {
      ret << song;
    }
  }
  return ret;
}

Song ASXParser::ParseTrack(QXmlStreamReader* reader, const QDir& dir) const {
  QString title, artist, album, ref;

  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    switch (type) {
      case QXmlStreamReader::StartElement: {
        QStringRef name = reader->name();
        if (name == "ref") {
          ref = reader->attributes().value("href").toString();
        } else if (name == "title") {
          title = reader->readElementText();
        } else if (name == "author") {
          artist = reader->readElementText();
        }
        break;
      }
      case QXmlStreamReader::EndElement: {
        if (reader->name() == "entry") {
          goto return_song;
        }
        break;
      }
      default:
        break;
    }
  }

return_song:
  Song song = LoadSong(ref, 0, dir);

  // Override metadata with what was in the playlist
  song.set_title(title);
  song.set_artist(artist);
  song.set_album(album);
  return song;
}

void ASXParser::Save(const SongList& songs, QIODevice* device,
                     const QDir&, Playlist::Path path_type) const {
  QXmlStreamWriter writer(device);
  writer.setAutoFormatting(true);
  writer.setAutoFormattingIndent(2);
  writer.writeStartDocument();
  {
    StreamElement asx("asx", &writer);
    writer.writeAttribute("version", "3.0");
    for (const Song& song : songs) {
      StreamElement entry("entry", &writer);
      writer.writeTextElement("title", song.title());
      {
        StreamElement ref("ref", &writer);
        writer.writeAttribute("href", song.url().toString());
      }
      if (!song.artist().isEmpty()) {
        writer.writeTextElement("author", song.artist());
      }
    }
  }
  writer.writeEndDocument();
}

bool ASXParser::TryMagic(const QByteArray& data) const {
  return data.toLower().contains("<asx");
}
