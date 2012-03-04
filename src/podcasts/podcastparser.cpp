/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "podcastparser.h"
#include "core/utilities.h"

#include <QDateTime>
#include <QXmlStreamReader>

const char* PodcastParser::kAtomNamespace = "http://www.w3.org/2005/Atom";
const char* PodcastParser::kItunesNamespace = "http://www.itunes.com/dtds/podcast-1.0.dtd";

PodcastParser::PodcastParser() {
}

Podcast PodcastParser::Load(QIODevice* device, const QUrl& url) const {
  Podcast ret;
  ret.set_url(url);

  QXmlStreamReader reader(device);
  if (!Utilities::ParseUntilElement(&reader, "rss") ||
      !Utilities::ParseUntilElement(&reader, "channel")) {
    return ret;
  }

  ParseChannel(&reader, &ret);
  return ret;
}

void PodcastParser::ParseChannel(QXmlStreamReader* reader, Podcast* ret) const {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
    case QXmlStreamReader::StartElement: {
      const QStringRef name = reader->name();
      if (name == "title") {
        ret->set_title(reader->readElementText());
      } else if (name == "link" && reader->namespaceUri().isEmpty()) {
        ret->set_link(QUrl(reader->readElementText()));
      } else if (name == "description") {
        ret->set_description(reader->readElementText());
      } else if (name == "owner" && reader->namespaceUri() == kItunesNamespace) {
        ParseItunesOwner(reader, ret);
      } else if (name == "image") {
        ParseImage(reader, ret);
      } else if (name == "copyright") {
        ret->set_copyright(reader->readElementText());
      } else if (name == "link" && reader->namespaceUri() == kAtomNamespace) {
        if (ret->url().isEmpty() && reader->attributes().value("rel") == "self") {
          ret->set_url(QUrl(reader->readElementText()));
        }
      } else {
        Utilities::ConsumeCurrentElement(reader);
      }
      break;
    }

    case QXmlStreamReader::EndElement:
      return;

    default:
      break;
    }
  }
}

void PodcastParser::ParseImage(QXmlStreamReader* reader, Podcast* ret) const {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
    case QXmlStreamReader::StartElement: {
      const QStringRef name = reader->name();
      if (name == "url") {
        ret->set_image_url(QUrl(reader->readElementText()));
      } else {
        Utilities::ConsumeCurrentElement(reader);
      }
      break;
    }

    case QXmlStreamReader::EndElement:
      return;

    default:
      break;
    }
  }
}

void PodcastParser::ParseItunesOwner(QXmlStreamReader* reader, Podcast* ret) const {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
    case QXmlStreamReader::StartElement: {
      const QStringRef name = reader->name();
      if (name == "name") {
        ret->set_owner_name(reader->readElementText());
      } else if (name == "email") {
        ret->set_owner_email(reader->readElementText());
      } else {
        Utilities::ConsumeCurrentElement(reader);
      }
      break;
    }

    case QXmlStreamReader::EndElement:
      return;

    default:
      break;
    }
  }
}

void PodcastParser::ParseItem(QXmlStreamReader* reader, Podcast* ret) const {
  PodcastEpisode episode;

  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
    case QXmlStreamReader::StartElement: {
      const QStringRef name = reader->name();
      if (name == "title") {
        episode.set_title(reader->readElementText());
      } else if (name == "description") {
        episode.set_description(reader->readElementText());
      } else if (name == "pubDate") {
        episode.set_publication_date(Utilities::ParseRFC822DateTime(reader->readElementText()));
      } else if (name == "duration" && reader->namespaceUri() == kItunesNamespace) {
        // http://www.apple.com/itunes/podcasts/specs.html
        QStringList parts = reader->readElementText().split(':');
        if (parts.count() == 2) {
          episode.set_duration_secs(parts[0].toInt() * 60 +
                                    parts[1].toInt());
        } else if (parts.count() >= 3) {
          episode.set_duration_secs(parts[0].toInt() * 60*60 +
                                    parts[1].toInt() * 60 +
                                    parts[2].toInt());
        }
      } else if (name == "enclosure") {
        if (reader->attributes().value("type").toString().startsWith("audio/")) {
          episode.set_url(QUrl(reader->attributes().value("href").toString()));
        }
      } else if (name == "author" && reader->namespaceUri() == kItunesNamespace) {
        episode.set_author(reader->readElementText());
      } else {
        Utilities::ConsumeCurrentElement(reader);
      }
      break;
    }

    case QXmlStreamReader::EndElement:
      if (!episode.url().isEmpty()) {
        ret->add_episode(episode);
      }
      return;

    default:
      break;
    }
  }
}
