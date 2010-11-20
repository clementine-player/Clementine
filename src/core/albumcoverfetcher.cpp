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
#include "network.h"

#include <QNetworkReply>
#include <QTimer>

#include <lastfm/Artist>
#include <lastfm/XmlQuery>
#include <lastfm/ws.h>

const int AlbumCoverFetcher::kMaxConcurrentRequests = 5;

AlbumCoverFetcher::AlbumCoverFetcher(QObject* parent, QNetworkAccessManager* network)
    : QObject(parent),
      network_(network ? network : new NetworkAccessManager(this)),
      next_id_(0),
      request_starter_(new QTimer(this))
{
  request_starter_->setInterval(1000);
  connect(request_starter_, SIGNAL(timeout()), SLOT(StartRequests()));
}

quint64 AlbumCoverFetcher::FetchAlbumCover(
    const QString& artist_name, const QString& album_name) {
  QueuedRequest request;
  request.query = artist_name + " " + album_name;
  request.search = false;
  request.id = next_id_ ++;

  AddRequest(request);
  return request.id;
}

quint64 AlbumCoverFetcher::SearchForCovers(const QString &query) {
  QueuedRequest request;
  request.query = query;
  request.search = true;
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

    QMap<QString, QString> params;
    params["method"] = "album.search";
    params["album"] = request.query;

    QNetworkReply* reply = lastfm::ws::post(params);
    connect(reply, SIGNAL(finished()), SLOT(AlbumSearchFinished()));
    active_requests_.insert(reply, request);
  }
}

void AlbumCoverFetcher::AlbumSearchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  QueuedRequest request = active_requests_.take(reply);

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: retry request.
    emit AlbumCoverFetched(request.id, QImage());
    return;
  }

  try {
    lastfm::XmlQuery query(lastfm::ws::parse(reply));
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif

    // Parse the list of search results
    QList<lastfm::XmlQuery> elements = query["results"]["albummatches"].children("album");
    SearchResults results;
    foreach (const lastfm::XmlQuery& element, elements) {
      SearchResult result;
      result.album = element["name"].text();
      result.artist = element["artist"].text();
      result.image_url = element["image size=large"].text();
      results << result;
    }

    // If we only wanted to do the search then we're done
    if (request.search) {
      emit SearchFinished(request.id, results);
      return;
    }

    // No results?
    if (results.isEmpty()) {
      emit AlbumCoverFetched(request.id, QImage());
      return;
    }

    // Now we need to fetch the first result's image
    QNetworkReply* image_reply = network_->get(QNetworkRequest(results[0].image_url));
    connect(image_reply, SIGNAL(finished()), SLOT(AlbumCoverFetchFinished()));

    active_requests_[image_reply] = request;
  } catch (std::runtime_error&) {
    if (request.search)
      emit SearchFinished(request.id, AlbumCoverFetcher::SearchResults());
    else
      emit AlbumCoverFetched(request.id, QImage());
  }
}

void AlbumCoverFetcher::AlbumCoverFetchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  QueuedRequest request = active_requests_.take(reply);

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: retry request.
    emit AlbumCoverFetched(request.id, QImage());
    return;
  }

  QImage image;
  image.loadFromData(reply->readAll());

  emit AlbumCoverFetched(request.id, image);
}
