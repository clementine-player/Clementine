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

#include "lastfmcoverprovider.h"

#include <QNetworkReply>

#include "albumcoverfetcher.h"
#include "coverprovider.h"
#include "core/closure.h"
#include "internet/lastfmcompat.h"

LastFmCoverProvider::LastFmCoverProvider(QObject* parent)
    : CoverProvider("last.fm", parent) {}

bool LastFmCoverProvider::StartSearch(const QString& artist,
                                      const QString& album, int id) {
  QMap<QString, QString> params;
  params["method"] = "album.search";
  params["album"] = album + " " + artist;

  QNetworkReply* reply = lastfm::ws::post(params);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(QueryFinished(QNetworkReply*, int)), reply, id);

  return true;
}

void LastFmCoverProvider::QueryFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  CoverSearchResults results;

  lastfm::XmlQuery query(lastfm::compat::EmptyXmlQuery());
  if (lastfm::compat::ParseQuery(reply->readAll(), &query)) {
    // parse the list of search results
    QList<lastfm::XmlQuery> elements =
        query["results"]["albummatches"].children("album");

    foreach(const lastfm::XmlQuery & element, elements) {
      CoverSearchResult result;
      result.description =
          element["artist"].text() + " - " + element["name"].text();
      result.image_url = QUrl(element["image size=extralarge"].text());
      results << result;
    }
  } else {
    // Drop through and emit an empty list of results.
  }

  emit SearchFinished(id, results);
}
