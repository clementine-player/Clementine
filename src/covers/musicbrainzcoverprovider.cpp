/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "musicbrainzcoverprovider.h"

#include <algorithm>
#include <functional>

#include <QXmlStreamReader>
#include <QUrlQuery>

#include "core/closure.h"
#include "core/network.h"

using std::mem_fun;

namespace {

static const char* kReleaseSearchUrl = "https://musicbrainz.org/ws/2/release/";
static const char* kAlbumCoverUrl =
    "https://coverartarchive.org/release/%1/front";
}  // namespace

MusicbrainzCoverProvider::MusicbrainzCoverProvider(QObject* parent)
    : CoverProvider("MusicBrainz", parent),
      network_(new NetworkAccessManager(this)) {}

bool MusicbrainzCoverProvider::StartSearch(const QString& artist,
                                           const QString& album, int id) {
  // Find release information.
  QUrl url(kReleaseSearchUrl);
  QString query = QString("release:\"%1\" AND artist:\"%2\"")
                      .arg(album.trimmed().replace('"', "\\\""))
                      .arg(artist.trimmed().replace('"', "\\\""));
  QUrlQuery url_query;
  url_query.addQueryItem("query", query);
  url_query.addQueryItem("limit", "5");
  url.setQuery(url_query);
  QNetworkRequest request(url);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(ReleaseSearchFinished(QNetworkReply*, int)), reply, id);

  cover_names_[id] = QString("%1 - %2").arg(artist, album);
  return true;
}

void MusicbrainzCoverProvider::ReleaseSearchFinished(QNetworkReply* reply,
                                                     int id) {
  reply->deleteLater();

  QList<QString> releases;

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    QXmlStreamReader::TokenType type = reader.readNext();
    if (type == QXmlStreamReader::StartElement && reader.name() == "release") {
      QStringRef release_id = reader.attributes().value("id");
      if (!release_id.isEmpty()) {
        releases.append(release_id.toString());
      }
    }
  }

  for (const QString& release_id : releases) {
    QUrl url(QString(kAlbumCoverUrl).arg(release_id));
    QNetworkReply* reply = network_->head(QNetworkRequest(url));
    image_checks_.insert(id, reply);
    NewClosure(reply, SIGNAL(finished()), this, SLOT(ImageCheckFinished(int)),
               id);
  }
}

void MusicbrainzCoverProvider::ImageCheckFinished(int id) {
  QList<QNetworkReply*> replies = image_checks_.values(id);
  int finished_count = std::count_if(replies.constBegin(), replies.constEnd(),
                                     mem_fun(&QNetworkReply::isFinished));
  if (finished_count == replies.size()) {
    QString cover_name = cover_names_.take(id);
    QList<CoverSearchResult> results;
    for (QNetworkReply* reply : replies) {
      reply->deleteLater();
      if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() <
          400) {
        CoverSearchResult result;
        result.description = cover_name;
        result.image_url = reply->url();
        results.append(result);
      }
    }
    image_checks_.remove(id);
    emit SearchFinished(id, results);
  }
}

void MusicbrainzCoverProvider::CancelSearch(int id) {
  QList<QNetworkReply*> replies = image_checks_.values(id);
  for (QNetworkReply* reply : replies) {
    reply->abort();
    reply->deleteLater();
  }
  image_checks_.remove(id);
}
