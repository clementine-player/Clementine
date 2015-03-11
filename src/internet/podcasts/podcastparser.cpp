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
#include <tinyxml2.h>

#include "core/logging.h"
#include "core/utilities.h"
#include "opmlcontainer.h"

// Namespace constants must be lower case.
const char* PodcastParser::kAtomNamespace = "http://www.w3.org/2005/atom";
const char* PodcastParser::kItunesNamespace = "http://www.itunes.com/dtds/podcast-1.0.dtd";

PodcastParser::PodcastParser() {
  supported_mime_types_ << "application/rss+xml"
                        << "application/xml"
                        << "text/x-opml"
                        << "text/xml"
                        << "application/atom+xml";
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

QVariant PodcastParser::Load(QByteArray data, const QUrl& url) const {
  QTextStream device(&data);
  QXmlStreamReader reader(device.device());
  tinyxml2::XMLDocument doc;
  doc.Parse(data.data());

  const QString name(doc.RootElement()->Name());
  qLog(Debug) << "Name of the reader:" << name << "on url" <<url;
  if (name == "rss") {
    Podcast podcast;
    if (!ParseRss(data, &podcast)) {
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
  } else if (name == "feed") {
    Podcast podcast;
    if (!ParseFeed(data, &podcast)) {
      return QVariant();
    } else {
      podcast.set_url(url);
      return QVariant::fromValue(podcast);
    }
  }
  return QVariant();
}

bool PodcastParser::ParseRss(QByteArray xml_text, Podcast* ret) const {
  tinyxml2::XMLDocument doc;
  doc.Parse(xml_text.data());
  tinyxml2::XMLElement* feedElement = doc.RootElement()->FirstChildElement("channel");
  ret->set_title(findElement(feedElement, QStringList() << "title", "title"));
  ret->set_link(QUrl(findElement(feedElement, QStringList() << "link", "link")));
  ret->set_url(QUrl(findElement(feedElement, QStringList() << "itunes:new-feed-url", "url")));
  ret->set_description(findElement(feedElement, QStringList() << "description", "description"));
  ret->set_copyright(findElement(feedElement, QStringList() << "copyright", "copyright"));
  ret->set_image_url_large(QUrl(findElement(feedElement, "image", "url", "image")));
  ret->set_owner_name(findElement(feedElement, "itunes:owner", "itunes:name", "owner:name"));
  if (ret->owner_name().isEmpty()) {
    ret->set_owner_name(findElement(feedElement, QStringList() << "itunes:author", "owner:author"));
  }
  if (ret->owner_name().isEmpty()) {
    ret->set_owner_name(ret->title());
  }
  ret->set_owner_email(findElement(feedElement, "itunes:owner", "itunes:email", "owner:email"));
  tinyxml2::XMLElement* itemElement = feedElement->FirstChildElement("item");
  while (itemElement) {
    ParseItem(itemElement, ret);
    itemElement = itemElement->NextSiblingElement("item");
  }
  return true;
}

void PodcastParser::ParseItem(tinyxml2::XMLElement* itemElement, Podcast* ret) const {
  PodcastEpisode episode;
  episode.set_title(findElement(itemElement, QStringList() << "title", "item:title"));

  episode.set_description(findElement(itemElement, QStringList() << "description", "item:description"));
  QString date(findElement(itemElement, QStringList() << "pubDate", "item:pubdate"));
  episode.set_publication_date(Utilities::ParseRFC822DateTime(date));

  if (!episode.publication_date().isValid()) {
    qLog(Error) << "Unable to parse date:" << date
                << "Please submit it to "
                << "https://github.com/clementine-player/Clementine/issues/new?title="
                 + QUrl::toPercentEncoding(QString("[podcast] Unable to parse date: %1").arg(date));
    episode.set_publication_date(QDateTime::currentDateTime());
  }

  episode.set_author(findElement(itemElement, QStringList() << "itunes:author" << "author", "item:author"));
  if (episode.author().isEmpty()) {
    episode.set_author(ret->owner_name());
  }

  episode.set_url(QUrl::fromEncoded(findUrl(itemElement, QStringList() << "enclosure", "url", "item:url")));
  qLog(Debug) << "Episodes url:" << episode.url();
  if (episode.url().isEmpty()) {
    return;
  }

  QString duration(findElement(itemElement, QStringList() << "itunes:duration", "item:duration"));
  qint64 duration_episode = parseDuration(duration);
  qLog(Debug) << "Duratio of an episode:" << duration_episode << "from:" <<duration;
  episode.set_duration_secs(duration_episode);
  ret->add_episode(episode);
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
          podcast.set_title(attributes.value("text").toString());
          podcast.set_image_url_large(QUrl::fromEncoded(
              attributes.value("imageHref").toString().toAscii()));
          podcast.set_url(QUrl::fromEncoded(
              attributes.value("xmlUrl").toString().toAscii()));
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

bool PodcastParser::ParseFeed(QByteArray xml_text, Podcast* ret) const {
  tinyxml2::XMLDocument doc;
  doc.Parse(xml_text.data());
  tinyxml2::XMLElement* feedElement = doc.FirstChildElement("feed");
  ret->set_title(findElement(feedElement, QStringList() << "title", "title"));
  ret->set_link(QUrl(findElement(feedElement, QStringList() << "link", "href", "link:href")));
  ret->set_url(QUrl(findElement(feedElement, QStringList() << "id", "id")));
  tinyxml2::XMLElement* entryElement = feedElement->FirstChildElement("entry");
  while (entryElement) {
    ParseEntry(entryElement, ret);
    entryElement = entryElement->NextSiblingElement();
  }
  return true;
}

void PodcastParser::ParseEntry(tinyxml2::XMLElement* entryElement, Podcast* ret) const {
  PodcastEpisode episode;
  episode.set_title(findElement(entryElement, QStringList() << "title", "title"));
  episode.set_description(findElement(entryElement, QStringList() << "summary", "summary"));
  QString date(findElement(entryElement, QStringList() << "published", "published"));
  episode.set_publication_date(Utilities::ParseISO8601DateTime(date));

  if (!episode.publication_date().isValid()) {
    qLog(Error) << "Unable to parse date:" << date
                << "Please submit it to "
                << "https://github.com/clementine-player/Clementine/issues/new?title="
                 + QUrl::toPercentEncoding(QString("[podcast] Unable to parse date: %1").arg(date));
    episode.set_publication_date(QDateTime::currentDateTime());
  }

  episode.set_author(findElement(entryElement, "author", "name", "author:name"));

  tinyxml2::XMLElement* linkElement = entryElement->FirstChildElement("link");
  while (linkElement) {
    if (QString(linkElement->Attribute("type")).contains("audio/")) {
      episode.set_url(QUrl::fromEncoded(linkElement->Attribute("href")));
      break;
    }
    linkElement = linkElement->NextSiblingElement("link");
  }
  ret->add_episode(episode);
}

QString PodcastParser::findElement(tinyxml2::XMLElement* searchedElement, QStringList names, QString tag) const {
  for (QString name : names) {
    tinyxml2::XMLElement* tmpElement = searchedElement->FirstChildElement(name.toStdString().c_str());
    if (tmpElement) {
      QString ret(QString().fromUtf8(tmpElement->GetText()));
      return ret;
    }
  }
  qLog(Error) << "Setting tag " << tag << " failed";
  return QString();
}

QString PodcastParser::findElement(tinyxml2::XMLElement* searchedElement, QString namea, QString nameb, QString tag) const {
    tinyxml2::XMLElement* tmpElement = searchedElement->FirstChildElement(namea.toStdString().c_str());
    if (tmpElement) {
      tmpElement = tmpElement->FirstChildElement(nameb.toStdString().c_str());
        if (tmpElement) {
          QString ret(QString().fromUtf8(tmpElement->GetText()));
          return ret;
        }
    }
  qLog(Error) << "Setting tag " << tag << " failed";
  return QString();
}

QString PodcastParser::findElement(tinyxml2::XMLElement* searchedElement, QStringList names, QString attribute, QString tag) const {
  for (QString name : names) {
    tinyxml2::XMLElement* tmpElement = searchedElement->FirstChildElement(name.toStdString().c_str());
    if (tmpElement) {
      QString ret(QString().fromUtf8(tmpElement->Attribute(attribute.toStdString().c_str())));
      return ret;
    }
  }
  qLog(Error) << "Setting tag " << tag << " failed";
  return QString();
}

QByteArray PodcastParser::findUrl(tinyxml2::XMLElement* searchedElement, QStringList names, QString tag) const {
  for (QString name : names) {
    tinyxml2::XMLElement* tmpElement = searchedElement->FirstChildElement(name.toStdString().c_str());
    if (tmpElement) {
      QByteArray ret(tmpElement->GetText());
      return ret;
    }
  }
  qLog(Error) << "Setting tag " << tag << " failed";
  return QByteArray();
}

QByteArray PodcastParser::findUrl(tinyxml2::XMLElement* searchedElement, QStringList names, QString attribute, QString tag) const {
  for (QString name : names) {
    tinyxml2::XMLElement* tmpElement = searchedElement->FirstChildElement(name.toStdString().c_str());
    if (tmpElement) {
      QByteArray ret(tmpElement->Attribute(attribute.toStdString().c_str()));
      return ret;
    }
  }
  qLog(Error) << "Setting tag " << tag << " failed";
  return QByteArray();
}

qint64 PodcastParser::parseDuration(QString duration) const {
  QStringList durations = duration.split(':');
  QList<int> tab;
  tab << 3600 << 60 << 1;
  int nums = 2;
  qint64 ret = 0;
  for (int num = durations.count()-1; num >= 0; --num) {
    ret += durations[num].toInt()*tab[nums];
    --nums;
  }
  return ret;
}
