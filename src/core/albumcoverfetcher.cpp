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

#include "albumcoverfetcher.h"
#include "networkaccessmanager.h"

#include <QNetworkReply>
#include <QTimer>

#include <lastfm/Artist>
#include <lastfm/XmlQuery>
#include <lastfm/ws.h>

const int AlbumCoverFetcher::kMaxConcurrentRequests = 5;

AlbumCoverFetcher::AlbumCoverFetcher(NetworkAccessManager* network, QObject* parent)
    : QObject(parent),
      network_(network->network()),
      next_id_(0),
      request_starter_(new QTimer(this))
{
  request_starter_->setInterval(1000);
  connect(request_starter_, SIGNAL(timeout()), SLOT(StartRequests()));
}

quint64 AlbumCoverFetcher::FetchAlbumCover(
    const QString& artist_name, const QString& album_name) {
  QueuedRequest request;
  request.album = album_name;
  request.artist = artist_name;
  request.id = next_id_ ++;

  AddRequest(request);
  return request.id;
}

quint64 AlbumCoverFetcher::SearchForCovers(const QString &query) {
  QueuedRequest request;
  request.query = query;
  request.id = next_id_ ++;

  AddRequest(request);
  return request.id;
}

void AlbumCoverFetcher::AddRequest(QueuedRequest req) {
  queued_requests_.enqueue(req);

  if (!request_starter_->isActive())
    request_starter_->start();

  if (active_requests_.count() < kMaxConcurrentRequests)
    StartRequests();
}

void AlbumCoverFetcher::Clear() {
  queued_requests_.clear();
}

void AlbumCoverFetcher::StartRequests() {
  if (queued_requests_.isEmpty()) {
    request_starter_->stop();
    return;
  }

  while (!queued_requests_.isEmpty() &&
         active_requests_.count() < kMaxConcurrentRequests) {
    QueuedRequest request = queued_requests_.dequeue();

    if (request.query.isEmpty()) {
      lastfm::Artist artist(request.artist);
      lastfm::Album album(artist, request.album);

      QNetworkReply* reply = album.getInfo();
      connect(reply, SIGNAL(finished()), SLOT(AlbumGetInfoFinished()));
      active_requests_.insert(reply, request.id);
    } else {
      QMap<QString, QString> params;
      params["method"] = "album.search";
      params["album"] = request.query;

      QNetworkReply* reply = lastfm::ws::post(params);
      connect(reply, SIGNAL(finished()), SLOT(AlbumSearchFinished()));
      active_requests_.insert(reply, request.id);
    }
  }
}

void AlbumCoverFetcher::AlbumGetInfoFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  quint64 id = active_requests_.take(reply);

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: retry request.
    emit AlbumCoverFetched(id, QImage());
    return;
  }

  try {
    lastfm::XmlQuery query(lastfm::ws::parse(reply));

    QUrl image_url(query["album"]["image size=large"].text());
    QNetworkReply* image_reply = network_->get(QNetworkRequest(image_url));
    connect(image_reply, SIGNAL(finished()), SLOT(AlbumCoverFetchFinished()));

    active_requests_[image_reply] = id;
  } catch (std::runtime_error&) {
    emit AlbumCoverFetched(id, QImage());
  }
}

void AlbumCoverFetcher::AlbumSearchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  quint64 id = active_requests_.take(reply);

  if (reply->error() != QNetworkReply::NoError) {
    qDebug() << "Error" << reply->error();
    // TODO: retry request.
    emit SearchFinished(id, SearchResults());
    return;
  }

  try {
    lastfm::XmlQuery query(lastfm::ws::parse(reply));

    QList<lastfm::XmlQuery> elements = query["results"]["albummatches"].children("album");
    SearchResults results;
    foreach (const lastfm::XmlQuery& element, elements) {
      SearchResult result;
      result.album = element["name"].text();
      result.artist = element["artist"].text();
      result.image_url = element["image size=large"].text();
      results << result;
    }

    emit SearchFinished(id, results);
  } catch (std::runtime_error&) {
    qDebug() << "Parse error";
    emit SearchFinished(id, SearchResults());
  }
}

void AlbumCoverFetcher::AlbumCoverFetchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  quint64 id = active_requests_.take(reply);

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: retry request.
    emit AlbumCoverFetched(id, QImage());
    return;
  }

  QImage image;
  image.loadFromData(reply->readAll());

  emit AlbumCoverFetched(id, image);
}
