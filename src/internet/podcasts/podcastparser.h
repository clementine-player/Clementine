/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_PODCASTS_PODCASTPARSER_H_
#define INTERNET_PODCASTS_PODCASTPARSER_H_

#include <QStringList>
#include <tinyxml2.h>

#include "podcast.h"

class OpmlContainer;

class QXmlStreamReader;

// Reads XML data from a QIODevice.
// Returns either a Podcast or an OpmlContainer depending on what was inside
// the XML document.
class PodcastParser {
 public:
  PodcastParser();

  static const char* kAtomNamespace;
  static const char* kItunesNamespace;

  const QStringList& supported_mime_types() const {
    return supported_mime_types_;
  }
  bool SupportsContentType(const QString& content_type) const;

  // You should check the type of the returned QVariant to see whether it
  // contains a Podcast or an OpmlContainer.  If the QVariant isNull then an
  // error occurred parsing the XML.
  QVariant Load(QByteArray data, const QUrl& url) const;

  // Really quick test to see if some data might be supported.  Load() might
  // still return a null QVariant.
  bool TryMagic(const QByteArray& data) const;

 private:
  bool ParseRss(QByteArray xml_text, Podcast* ret) const;
  void ParseItem(tinyxml2::XMLElement* entryElement, Podcast* ret) const;

  bool ParseOpml(QXmlStreamReader* reader, OpmlContainer* ret) const;
  void ParseOutline(QXmlStreamReader* reader, OpmlContainer* ret) const;
  bool ParseFeed(QByteArray xml_text, Podcast* ret) const;
  void ParseEntry(tinyxml2::XMLElement* entryElement, Podcast* ret) const;
  QString findElement(tinyxml2::XMLElement* searchedElement, QStringList names, QString tag) const;
  QString findElement(tinyxml2::XMLElement* searchedElement, QString namea, QString namfb, QString tag) const;
  QString findElement(tinyxml2::XMLElement* searchedElement, QStringList names, QString attribute, QString tag) const;
  QByteArray findUrl(tinyxml2::XMLElement* searchedElement, QStringList names, QString tag) const;
  QByteArray findUrl(tinyxml2::XMLElement* searchedElement, QStringList names, QString attribute, QString tag) const;
  qint64 parseDuration(QString duration) const;

 private:
  QStringList supported_mime_types_;
};

#endif  // INTERNET_PODCASTS_PODCASTPARSER_H_
