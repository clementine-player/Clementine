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

#include "albumcoverfetcher.h"
#include "coverprovider.h"
#include "lastfmcoverprovider.h"

#include <lastfm/Artist>
#include <lastfm/XmlQuery>
#include <lastfm/ws.h>

#include <QNetworkReply>

LastFmCoverProvider::LastFmCoverProvider(QObject* parent)
  : CoverProvider("last.fm", parent)
{
}

QNetworkReply* LastFmCoverProvider::SendRequest(const QString& query) {
  QMap<QString, QString> params;
  params["method"] = "album.search";
  params["album"] = query;

  return lastfm::ws::post(params);
}

CoverSearchResults LastFmCoverProvider::ParseReply(QNetworkReply* reply) {
  CoverSearchResults results;

  try {
    lastfm::XmlQuery query(lastfm::ws::parse(reply));
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      return results;
#endif

    // parse the list of search results
    QList<lastfm::XmlQuery> elements = query["results"]["albummatches"].children("album");

    foreach (const lastfm::XmlQuery& element, elements) {
      CoverSearchResult result;
      result.description = element["artist"].text() + " - " + element["name"].text();
      result.image_url = element["image size=extralarge"].text();
      results << result;
    }

    return results;

  } catch(std::runtime_error&) {
    return results;
  }
}
