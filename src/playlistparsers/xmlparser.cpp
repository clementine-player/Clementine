/* This file is part of Clementine.

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

#include "xmlparser.h"

#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QUrl>
#include <QXmlStreamReader>

XMLParser::XMLParser(QObject* parent)
    : ParserBase(parent) {
}

bool XMLParser::ParseUntilElement(QXmlStreamReader* reader, const QString& name) const {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement:
        if (reader->name() == name) {
          return true;
        }
        break;
      default:
        break;
    }
  }
  return false;
}

void XMLParser::IgnoreElement(QXmlStreamReader* reader) const {
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

QString XMLParser::MakeRelativeTo(const QString& filename_or_url, const QDir& dir) const {
  QString file = ParserBase::MakeRelativeTo(filename_or_url, dir);
  if (!file.contains(QRegExp("^[a-z]+://"))) {
    return QUrl::fromLocalFile(file).toString();
  }
  return file;
}
