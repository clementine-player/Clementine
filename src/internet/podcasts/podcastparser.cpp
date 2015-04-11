/* This file is part of Clementine.
   Copyright 2012, 2014, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include <QDateTime>
#include <QXmlStreamReader>

#include "core/logging.h"
#include "core/utilities.h"
#include "opmlcontainer.h"

// Namespace constants must be lower case.
const char* PodcastParser::kAtomNamespace = "http://www.w3.org/2005/atom";
const char* PodcastParser::kItunesNamespace =
    "http://www.itunes.com/dtds/podcast-1.0.dtd";

PodcastParser::PodcastParser() {
  supported_mime_types_ << "application/rss+xml"
                        << "application/xml"
                        << "text/x-opml"
                        << "text/xml";
}

bool PodcastParser::SupportsContentType(const QString& content_type) const {
  if (content_type.isEmpty()) {
    // Why not have a go.
    return true;
  }

  for (const QString& mime_type : supported_mime_types()) {
    if (content_type.contains(mime_type)) {
      return true;
    }
  }
  return false;
}

bool PodcastParser::TryMagic(const QByteArray& data) const {
  QString str(QString::fromUtf8(data));
  return str.contains(QRegExp("<rss\\b")) || str.contains(QRegExp("<opml\\b"));
}

QVariant PodcastParser::Load(QIODevice* device, const QUrl& url) const {
  QXmlStreamReader reader(device);

  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::StartElement: {
        const QStringRef name = reader.name();
        if (name == "rss") {
          Podcast podcast;
          if (!ParseRss(&reader, &podcast)) {
            return QVariant();
          } else {
            podcast.set_url(url);
            return QVariant::fromValue(podcast);
          }
        } else if (name == "opml") {
          OpmlContainer container;
          if (!ParseOpml(&reader, &container)) {
            return QVariant();
          } else {
            container.url = url;
            return QVariant::fromValue(container);
          }
        }

        return QVariant();
      }

      default:
        break;
    }
  }

  return QVariant();
}

bool PodcastParser::ParseRss(QXmlStreamReader* reader, Podcast* ret) const {
  if (!Utilities::ParseUntilElement(reader, "channel")) {
    return false;
  }

  ParseChannel(reader, ret);
  return true;
}

void PodcastParser::ParseChannel(QXmlStreamReader* reader, Podcast* ret) const {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement: {
        const QStringRef name = reader->name();
        const QString lower_namespace =
            reader->namespaceUri().toString().toLower();

        if (name == "title") {
          ret->set_title(reader->readElementText());
        } else if (name == "link" && lower_namespace.isEmpty()) {
          ret->set_link(QUrl::fromEncoded(reader->readElementText().toLatin1()));
        } else if (name == "description") {
          ret->set_description(reader->readElementText());
        } else if (name == "owner" && lower_namespace == kItunesNamespace) {
          ParseItunesOwner(reader, ret);
        } else if (name == "image") {
          ParseImage(reader, ret);
        } else if (name == "copyright") {
          ret->set_copyright(reader->readElementText());
        } else if (name == "link" && lower_namespace == kAtomNamespace &&
                   ret->url().isEmpty() &&
                   reader->attributes().value("rel") == "self") {
          ret->set_url(QUrl::fromEncoded(reader->readElementText().toLatin1()));
        } else if (name == "item") {
          ParseItem(reader, ret);
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
          ret->set_image_url_large(
              QUrl::fromEncoded(reader->readElementText().toLatin1()));
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

void PodcastParser::ParseItunesOwner(QXmlStreamReader* reader,
                                     Podcast* ret) const {
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
        const QString lower_namespace =
            reader->namespaceUri().toString().toLower();

        if (name == "title") {
          episode.set_title(reader->readElementText());
        } else if (name == "description") {
          episode.set_description(reader->readElementText());
        } else if (name == "pubDate") {
          QString date = reader->readElementText();
          episode.set_publication_date(Utilities::ParseRFC822DateTime(date));
          if (!episode.publication_date().isValid()) {
            qLog(Error) << "Unable to parse date:" << date
                        << "Please submit it to "
                        << "https://github.com/clementine-player/Clementine/issues/new?title="
                         + QUrl::toPercentEncoding(QString("[podcast] Unable to parse date: %1").arg(date));
          }
        } else if (name == "duration" && lower_namespace == kItunesNamespace) {
          // http://www.apple.com/itunes/podcasts/specs.html
          QStringList parts = reader->readElementText().split(':');
          if (parts.count() == 2) {
            episode.set_duration_secs(parts[0].toInt() * 60 + parts[1].toInt());
          } else if (parts.count() >= 3) {
            episode.set_duration_secs(parts[0].toInt() * 60 * 60 +
                                      parts[1].toInt() * 60 + parts[2].toInt());
          }
        } else if (name == "enclosure") {
          const QString type = reader->attributes().value("type").toString();
          if (type.startsWith("audio/") || type.startsWith("x-audio/")) {
            episode.set_url(QUrl::fromEncoded(
                reader->attributes().value("url").toString().toLatin1()));
          }
          Utilities::ConsumeCurrentElement(reader);
        } else if (name == "author" && lower_namespace == kItunesNamespace) {
          episode.set_author(reader->readElementText());
        } else {
          Utilities::ConsumeCurrentElement(reader);
        }
        break;
      }

      case QXmlStreamReader::EndElement:
        if (!episode.publication_date().isValid()) {
          episode.set_publication_date(QDateTime::currentDateTime());
        }
        if (!episode.url().isEmpty()) {
          ret->add_episode(episode);
        }
        return;

      default:
        break;
    }
  }
}

bool PodcastParser::ParseOpml(QXmlStreamReader* reader,
                              OpmlContainer* ret) const {
  if (!Utilities::ParseUntilElement(reader, "body")) {
    return false;
  }

  ParseOutline(reader, ret);

  // OPML files sometimes consist of a single top level container.
  while (ret->feeds.count() == 0 && ret->containers.count() == 1) {
    *ret = ret->containers[0];
  }

  return true;
}

void PodcastParser::ParseOutline(QXmlStreamReader* reader,
                                 OpmlContainer* ret) const {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement: {
        const QStringRef name = reader->name();
        if (name != "outline") {
          Utilities::ConsumeCurrentElement(reader);
          continue;
        }

        QXmlStreamAttributes attributes = reader->attributes();

        if (attributes.value("type").toString() == "rss") {
          // Parse the feed and add it to this container
          Podcast podcast;
          podcast.set_description(attributes.value("description").toString());

          QString title = attributes.value("title").toString();
          if (title.isEmpty()) {
            title = attributes.value("text").toString();
          }
          podcast.set_title(title);
          podcast.set_image_url_large(QUrl::fromEncoded(
              attributes.value("imageHref").toString().toLatin1()));
          podcast.set_url(QUrl::fromEncoded(
              attributes.value("xmlUrl").toString().toLatin1()));
          ret->feeds.append(podcast);

          // Consume any children and the EndElement.
          Utilities::ConsumeCurrentElement(reader);
        } else {
          // Create a new child container
          OpmlContainer child;

          // Take the name from the fullname attribute first if it exists.
          child.name = attributes.value("fullname").toString();
          if (child.name.isEmpty()) {
            child.name = attributes.value("text").toString();
          }

          // Parse its contents and add it to this container
          ParseOutline(reader, &child);
          ret->containers.append(child);
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
