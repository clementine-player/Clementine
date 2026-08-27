/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "lastfmxmlquery.h"

#include <QDomDocument>

bool LastFmXmlQuery::Parse(const QByteArray& data, bool* connection_problem) {
  QDomDocument doc;
  if (!doc.setContent(data)) {
    if (connection_problem) *connection_problem = true;
    return false;
  }

  element_ = doc.documentElement();
  if (connection_problem) *connection_problem = false;

  return element_.attribute("status") != "failed";
}

LastFmXmlQuery LastFmXmlQuery::operator[](const QString& selector) const {
  const int space = selector.indexOf(' ');
  if (space == -1) {
    return LastFmXmlQuery(element_.firstChildElement(selector));
  }

  // "image size=extralarge" -> tag "image" with attribute size=extralarge.
  const QString tag = selector.left(space);
  const QString predicate = selector.mid(space + 1);
  const int eq = predicate.indexOf('=');
  const QString attr_name = predicate.left(eq);
  const QString attr_value = predicate.mid(eq + 1);

  for (QDomElement child = element_.firstChildElement(tag); !child.isNull();
       child = child.nextSiblingElement(tag)) {
    if (child.attribute(attr_name) == attr_value) {
      return LastFmXmlQuery(child);
    }
  }
  return LastFmXmlQuery(QDomElement());
}

QList<LastFmXmlQuery> LastFmXmlQuery::children(const QString& tag_name) const {
  QList<LastFmXmlQuery> ret;
  for (QDomElement child = element_.firstChildElement(tag_name);
       !child.isNull(); child = child.nextSiblingElement(tag_name)) {
    ret << LastFmXmlQuery(child);
  }
  return ret;
}

QString LastFmXmlQuery::text() const { return element_.text(); }
