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
#include "coverprovider.h"
#include "coverproviders.h"
#include "core/logging.h"

#include <QMutexLocker>
#include <QNetworkReply>
#include <QTimer>
#include <QtDebug>

const int AlbumCoverFetcherSearch::kSearchTimeout = 10000;

AlbumCoverFetcherSearch::AlbumCoverFetcherSearch(const CoverSearchRequest& request,
                                                 QNetworkAccessManager* network,
                                                 QObject* parent)
  : QObject(parent),
    request_(request),
    network_(network)
{
  // we will terminate the search after kSearchTimeout miliseconds if we are not
  // able to find all of the results before that point in time
  QTimer::singleShot(kSearchTimeout, this, SLOT(Timeout()));
}

void AlbumCoverFetcherSearch::Timeout() {
  TerminateSearch();
}

void AlbumCoverFetcherSearch::TerminateSearch() {
  foreach (QNetworkReply* reply, pending_requests_.keys()) {
    disconnect(reply, SIGNAL(finished()), this, SLOT(ProviderSearchFinished()));
    reply->abort();
  }

  if(request_.search) {
    // send everything we've managed to find
    emit SearchFinished(request_.id, results_);
  } else {
    emit AlbumCoverFetched(request_.id, QImage());
  }
}

void AlbumCoverFetcherSearch::Start() {
  // end this search before it even began if there are no providers...
  foreach(CoverProvider* provider, CoverProviders::instance().List(this)) {
    QNetworkReply* reply = provider->SendRequest(request_.query);

    if (reply) {
      connect(reply, SIGNAL(finished()), SLOT(ProviderSearchFinished()));
      pending_requests_.insert(reply, provider);
    }
  }

  if(pending_requests_.isEmpty()) {
    TerminateSearch();
  }
}

void AlbumCoverFetcherSearch::ProviderSearchFinished() {
  // Note: we don't delete the reply here.  It's parented to the provider's
  // network access manager which is deleted when it is deleted.
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

  CoverProvider* provider = pending_requests_.take(reply);

  if(reply->error() == QNetworkReply::NoError) {
    CoverSearchResults partial_results = provider->ParseReply(reply);

    // Add categories to the results if the provider didn't specify them
    for (int i=0 ; i<partial_results.count() ; ++i) {
      if (partial_results[i].category.isEmpty()) {
        partial_results[i].category = provider->name();
      }
    }

    // add results from the current provider to our pool
    results_.append(partial_results);
  } else {
    QString contents(reply->readAll());
    qLog(Debug) << "CoverProvider's request error - summary:";
    qLog(Debug) << reply->errorString();
    qLog(Debug) << "CoverProvider's request error - contents:";
    qLog(Debug) << contents;
  }

  // do we have more providers left?
  if(!pending_requests_.isEmpty()) {
    return;
  }

  // if we only wanted to do the search then we're done
  if (request_.search) {
    emit SearchFinished(request_.id, results_);
    return;
  }

  // no results?
  if (results_.isEmpty()) {
    emit AlbumCoverFetched(request_.id, QImage());
    return;
  }

  // now we need to fetch the first result's image
  QNetworkReply* image_reply = network_->get(QNetworkRequest(results_[0].image_url));
  connect(image_reply, SIGNAL(finished()), SLOT(ProviderCoverFetchFinished()));
}

void AlbumCoverFetcherSearch::ProviderCoverFetchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: retry request.
    emit AlbumCoverFetched(request_.id, QImage());

  } else {
    QImage image;
    image.loadFromData(reply->readAll());

    emit AlbumCoverFetched(request_.id, image);
  }
}
