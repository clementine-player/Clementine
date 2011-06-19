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

bool LastFmCoverProvider::StartSearch(const QString& query, int id) {
  QMap<QString, QString> params;
  params["method"] = "album.search";
  params["album"] = query;

  QNetworkReply* reply = lastfm::ws::post(params);
  connect(reply, SIGNAL(finished()), SLOT(QueryFinished()));
  pending_queries_[reply] = id;

  return true;
}

void LastFmCoverProvider::QueryFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply || !pending_queries_.contains(reply))
    return;

  int id = pending_queries_.take(reply);
  reply->deleteLater();

  CoverSearchResults results;

  try {
    lastfm::XmlQuery query(lastfm::ws::parse(reply));
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError) {
      throw std::runtime_error();
    }
#endif

    // parse the list of search results
    QList<lastfm::XmlQuery> elements = query["results"]["albummatches"].children("album");

    foreach (const lastfm::XmlQuery& element, elements) {
      CoverSearchResult result;
      result.description = element["artist"].text() + " - " + element["name"].text();
      result.image_url = element["image size=extralarge"].text();
      results << result;
    }
  } catch(std::runtime_error&) {
    // Drop through and emit an empty list of results.
  }

  emit SearchFinished(id, results);
}
