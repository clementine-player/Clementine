#include "xspfparser.h"

#include <QFile>
#include <QIODevice>
#include <QUrl>
#include <QXmlStreamReader>

XSPFParser::XSPFParser(QIODevice* device, QObject* parent)
    : QObject(parent),
      device_(device) {
}

const SongList& XSPFParser::Parse() {
  QXmlStreamReader reader(device_);
  if (!ParseUntilElement(&reader, "playlist") ||
      !ParseUntilElement(&reader, "trackList")) {
    return songs_;
  }

  while (!reader.atEnd() && ParseUntilElement(&reader, "track")) {
    Song song = ParseTrack(&reader);
    if (song.is_valid()) {
      songs_ << song;
    }
  }
  return songs_;
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
            return Song();
          }
          song.set_length(length);
        } else if (name == "image") {
          // TODO: Fetch album covers.
        } else if (name == "info") {
          // TODO: Do something with extra info?
        }
        break;
      }
      default:
        break;
    }
  }
  song.Init(title, artist, album, length);
  return song;
}
