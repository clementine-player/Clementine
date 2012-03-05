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

class QXmlStreamReader;

// Reads XML data from a QIODevice and returns a Podcast.
class PodcastParser {
public:
  PodcastParser();

  static const char* kAtomNamespace;
  static const char* kItunesNamespace;

  const QStringList& supported_mime_types() const { return supported_mime_types_; }
  bool SupportsContentType(const QString& content_type) const;

  bool Load(QIODevice* device, const QUrl& url, Podcast* ret) const;

private:
  void ParseChannel(QXmlStreamReader* reader, Podcast* ret) const;
  void ParseImage(QXmlStreamReader* reader, Podcast* ret) const;
  void ParseItunesOwner(QXmlStreamReader* reader, Podcast* ret) const;
  void ParseItem(QXmlStreamReader* reader, Podcast* ret) const;

private:
  QStringList supported_mime_types_;
};

#endif // PODCASTPARSER_H
