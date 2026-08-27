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

#ifndef INTERNET_LASTFM_LASTFMXMLQUERY_H_
#define INTERNET_LASTFM_LASTFMXMLQUERY_H_

#include <QDomElement>
#include <QList>
#include <QString>

// A small QDomDocument-based wrapper around Last.fm's XML API responses,
// e.g.:
//   <lfm status="ok">
//     <session>
//       <name>example</name>
//       <key>abc123</key>
//     </session>
//   </lfm>
//
// Supports the chained element lookup and attribute-predicate syntax the
// call sites use, e.g. query["session"]["name"].text() or
// element["image size=extralarge"].text() (the latter selects the first
// <image size="extralarge">...</image> child, since Last.fm responses often
// have several same-named siblings distinguished only by an attribute).
class LastFmXmlQuery {
 public:
  LastFmXmlQuery() = default;

  // Parses a raw Last.fm API response. Returns false on malformed XML or if
  // the root element's status attribute is "failed". If connection_problem
  // is non-null, it's set to true only for malformed XML (as opposed to a
  // well-formed error response), since that's the case worth distinguishing
  // to the user.
  bool Parse(const QByteArray& data, bool* connection_problem = nullptr);

  // Looks up a child element, either by tag name ("session") or by tag name
  // plus an attribute predicate ("image size=extralarge"). Returns a
  // null/empty LastFmXmlQuery if no such child exists.
  LastFmXmlQuery operator[](const QString& selector) const;

  // All direct child elements with the given tag name.
  QList<LastFmXmlQuery> children(const QString& tag_name) const;

  // The concatenated text of this element's direct text-node children.
  QString text() const;

  // True if this wraps no element (e.g. a failed lookup).
  bool isEmpty() const { return element_.isNull(); }

 private:
  explicit LastFmXmlQuery(const QDomElement& element) : element_(element) {}

  QDomElement element_;
};

#endif  // INTERNET_LASTFM_LASTFMXMLQUERY_H_
