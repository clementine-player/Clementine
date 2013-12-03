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

#include "ultimatelyricsprovider.h"
#include "ultimatelyricsreader.h"
#include "core/logging.h"

#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamReader>

UltimateLyricsReader::UltimateLyricsReader(QObject* parent)
  : QObject(parent),
    thread_(qApp->thread())
{
}

void UltimateLyricsReader::SetThread(QThread *thread) {
  thread_ = thread;
}

QList<SongInfoProvider*> UltimateLyricsReader::Parse(const QString& filename) const {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    qLog(Warning) << "Error opening" << filename;
    return QList<SongInfoProvider*>();
  }

  return ParseDevice(&file);
}

QList<SongInfoProvider*> UltimateLyricsReader::ParseDevice(QIODevice* device) const {
  QList<SongInfoProvider*> ret;

  QXmlStreamReader reader(device);
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.name() == "provider") {
      SongInfoProvider* provider = ParseProvider(&reader);
      if (provider) {
        provider->moveToThread(thread_);
        ret << provider;
      }
    }
  }

  return ret;
}

SongInfoProvider* UltimateLyricsReader::ParseProvider(QXmlStreamReader* reader) const {
  QXmlStreamAttributes attributes = reader->attributes();

  UltimateLyricsProvider* scraper = new UltimateLyricsProvider;
  scraper->set_name(attributes.value("name").toString());
  scraper->set_title(attributes.value("title").toString());
  scraper->set_charset(attributes.value("charset").toString());
  scraper->set_url(attributes.value("url").toString());

  while (!reader->atEnd()) {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::EndElement)
      break;

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      if (reader->name() == "extract")
        scraper->add_extract_rule(ParseRule(reader));
      else if (reader->name() == "exclude")
        scraper->add_exclude_rule(ParseRule(reader));
      else if (reader->name() == "invalidIndicator")
        scraper->add_invalid_indicator(ParseInvalidIndicator(reader));
      else if (reader->name() == "urlFormat") {
        scraper->add_url_format(reader->attributes().value("replace").toString(),
                                reader->attributes().value("with").toString());
        reader->skipCurrentElement();
      }
      else
        reader->skipCurrentElement();
    }
  }
  return scraper;
}

UltimateLyricsProvider::Rule UltimateLyricsReader::ParseRule(QXmlStreamReader* reader) const {
  UltimateLyricsProvider::Rule ret;

  while (!reader->atEnd()) {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::EndElement)
      break;

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      if (reader->name() == "item") {
        QXmlStreamAttributes attr = reader->attributes();
        if (attr.hasAttribute("tag"))
          ret << UltimateLyricsProvider::RuleItem(attr.value("tag").toString(), QString());
        else if (attr.hasAttribute("url"))
          ret << UltimateLyricsProvider::RuleItem(attr.value("url").toString(), QString());
        else if (attr.hasAttribute("begin"))
          ret << UltimateLyricsProvider::RuleItem(attr.value("begin").toString(),
                                                  attr.value("end").toString());
      }
      reader->skipCurrentElement();
    }
  }
  return ret;
}

QString UltimateLyricsReader::ParseInvalidIndicator(QXmlStreamReader* reader) const {
  QString ret = reader->attributes().value("value").toString();
  reader->skipCurrentElement();
  return ret;
}

