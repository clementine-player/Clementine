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
#include "albumcoverfetchersearch.h"
#include "core/network.h"

#include <QTimer>

const int AlbumCoverFetcher::kMaxConcurrentRequests = 5;

AlbumCoverFetcher::AlbumCoverFetcher(CoverProviders* cover_providers,
                                     QObject* parent,
                                     QNetworkAccessManager* network)
    : QObject(parent),
      cover_providers_(cover_providers),
      network_(network ? network : new NetworkAccessManager(this)),
      next_id_(0),
      request_starter_(new QTimer(this)) {
  request_starter_->setInterval(1000);
  connect(request_starter_, SIGNAL(timeout()), SLOT(StartRequests()));
}

quint64 AlbumCoverFetcher::FetchAlbumCover(const QString& artist,
                                           const QString& album) {
  CoverSearchRequest request;
  request.artist = artist;
  request.album = album;
  request.search = false;
  request.id = next_id_++;

  AddRequest(request);
  return request.id;
}

quint64 AlbumCoverFetcher::SearchForCovers(const QString& artist,
                                           const QString& album) {
  CoverSearchRequest request;
  request.artist = artist;
  request.album = album;
  request.search = true;
  request.id = next_id_++;

  AddRequest(request);
  return request.id;
}

void AlbumCoverFetcher::AddRequest(const CoverSearchRequest& req) {
  queued_requests_.enqueue(req);

  if (!request_starter_->isActive()) request_starter_->start();

  if (active_requests_.size() < kMaxConcurrentRequests) StartRequests();
}

void AlbumCoverFetcher::Clear() {
  queued_requests_.clear();

  foreach(AlbumCoverFetcherSearch * search, active_requests_.values()) {
    search->Cancel();
    search->deleteLater();
  }
  active_requests_.clear();
}

void AlbumCoverFetcher::StartRequests() {
  if (queued_requests_.isEmpty()) {
    request_starter_->stop();
    return;
  }

  while (!queued_requests_.isEmpty() &&
         active_requests_.size() < kMaxConcurrentRequests) {

    CoverSearchRequest request = queued_requests_.dequeue();

    // search objects are this fetcher's children so worst case scenario - they
    // get
    // deleted with it
    AlbumCoverFetcherSearch* search =
        new AlbumCoverFetcherSearch(request, network_, this);
    active_requests_.insert(request.id, search);

    connect(search, SIGNAL(SearchFinished(quint64, CoverSearchResults)),
            SLOT(SingleSearchFinished(quint64, CoverSearchResults)));
    connect(search, SIGNAL(AlbumCoverFetched(quint64, const QImage&)),
            SLOT(SingleCoverFetched(quint64, const QImage&)));

    search->Start(cover_providers_);
  }
}

void AlbumCoverFetcher::SingleSearchFinished(quint64 request_id,
                                             CoverSearchResults results) {
  AlbumCoverFetcherSearch* search = active_requests_.take(request_id);
  if (!search) return;

  search->deleteLater();
  emit SearchFinished(request_id, results, search->statistics());
}

void AlbumCoverFetcher::SingleCoverFetched(quint64 request_id,
                                           const QImage& image) {
  AlbumCoverFetcherSearch* search = active_requests_.take(request_id);
  if (!search) return;

  search->deleteLater();
  emit AlbumCoverFetched(request_id, image, search->statistics());
}
