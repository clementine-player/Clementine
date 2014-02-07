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

#ifndef PODCASTPARSER_H
#define PODCASTPARSER_H

#include <QStringList>

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
  QVariant Load(QIODevice* device, const QUrl& url) const;

  // Really quick test to see if some data might be supported.  Load() might
  // still return a null QVariant.
  bool TryMagic(const QByteArray& data) const;

 private:
  bool ParseRss(QXmlStreamReader* reader, Podcast* ret) const;
  void ParseChannel(QXmlStreamReader* reader, Podcast* ret) const;
  void ParseImage(QXmlStreamReader* reader, Podcast* ret) const;
  void ParseItunesOwner(QXmlStreamReader* reader, Podcast* ret) const;
  void ParseItem(QXmlStreamReader* reader, Podcast* ret) const;

  bool ParseOpml(QXmlStreamReader* reader, OpmlContainer* ret) const;
  void ParseOutline(QXmlStreamReader* reader, OpmlContainer* ret) const;

 private:
  QStringList supported_mime_types_;
};

#endif  // PODCASTPARSER_H
