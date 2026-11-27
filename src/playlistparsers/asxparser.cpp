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

#include <QBuffer>
#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QRegularExpression>
#include <QUrl>
#include <QXmlStreamReader>
#include <QtDebug>

#include "core/utilities.h"

ASXParser::ASXParser(LibraryBackendInterface* library, QObject* parent)
    : XMLParser(library, parent) {}

SongList ASXParser::Load(QIODevice* device, const QString& playlist_path,
                         const QDir& dir) const {
  // We have to load everything first so we can munge the "XML".
  QByteArray data = device->readAll();

  // (thanks Amarok...)
  // ASX looks a lot like xml, but doesn't require tags to be case sensitive,
  // meaning we have to accept things like: <Abstract>...</abstract>
  // Round-trip through Latin-1 so non-UTF-8 bytes (e.g. windows-1252) are
  // preserved unchanged while unquoted ASCII markup inside <...> is lowercased
  // in a single pass (preserving case-sensitive URLs inside quotes).
  QString text = QString::fromLatin1(data);
  bool in_tag = false;
  char quote_char = 0;
  for (qsizetype i = 0; i < text.size(); ++i) {
    const char16_t code = text[i].unicode();
    if (!in_tag) {
      if (code == u'<') in_tag = true;
    } else if (quote_char != 0) {
      if (code == static_cast<unsigned char>(quote_char)) quote_char = 0;
    } else if (code == u'"' || code == u'\'') {
      quote_char = static_cast<char>(code);
    } else {
      if (code == u'>') in_tag = false;
      if (code >= u'A' && code <= u'Z') {
        text[i] = QChar::fromLatin1(static_cast<char>(code - u'A' + u'a'));
      }
    }
  }

  // Some playlists have unescaped & characters in URLs :(  Match href="..."
  // with a word-boundary lookbehind and copy in one O(N) pass so repeated
  // in-place replacements don't go quadratic or corrupt offsets.
  static const QRegularExpression kHrefRe(
      "(?<![a-z0-9_-])(href\\s*=\\s*\")([^\"]*)\"");
  static const QRegularExpression kUnescapedAmpRe(
      "&(?!amp;|quot;|apos;|lt;|gt;)");
  QString escaped;
  escaped.reserve(text.size());
  qsizetype pos = 0;
  QRegularExpressionMatchIterator it = kHrefRe.globalMatch(text);
  while (it.hasNext()) {
    const QRegularExpressionMatch match = it.next();
    escaped.append(QStringView(text).sliced(pos, match.capturedStart() - pos));
    escaped.append(match.capturedView(1));
    QString url = match.captured(2);
    url.replace(kUnescapedAmpRe, "&amp;");
    escaped.append(url);
    escaped.append('"');
    pos = match.capturedEnd();
  }
  escaped.append(QStringView(text).sliced(pos));
  data = escaped.toLatin1();

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
        QStringView name = reader->name();
        if (name == QLatin1String("ref")) {
          ref = reader->attributes().value("href").toString();
        } else if (name == QLatin1String("title")) {
          title = reader->readElementText();
        } else if (name == QLatin1String("author")) {
          artist = reader->readElementText();
        }
        break;
      }
      case QXmlStreamReader::EndElement: {
        if (reader->name() == QLatin1String("entry")) {
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

void ASXParser::Save(const SongList& songs, QIODevice* device, const QDir&,
                     Playlist::Path path_type) const {
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
