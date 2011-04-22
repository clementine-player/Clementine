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
  startTimer(kSearchTimeout);
}

void AlbumCoverFetcherSearch::timerEvent(QTimerEvent* event) {
  Q_UNUSED(event);
  TerminateSearch();
}

void AlbumCoverFetcherSearch::TerminateSearch() {
  if(request_.search) {
    // send everything we've managed to find
    emit SearchFinished(request_.id, results_);
  } else {
    emit AlbumCoverFetched(request_.id, QImage());
  }
}

void AlbumCoverFetcherSearch::Start() {
  QList<CoverProvider*> providers_list = CoverProviders::instance().List();
  providers_left_ = providers_list.size();

  // end this search before it even began if there are no providers...
  if(!providers_left_) {
    TerminateSearch();
  } else {
    foreach(CoverProvider* provider, providers_list) {
      QNetworkReply* reply = provider->SendRequest(request_.query);

      connect(reply, SIGNAL(finished()), SLOT(ProviderSearchFinished()));
      providers_.insert(reply, provider);
    }
  }
}

void AlbumCoverFetcherSearch::ProviderSearchFinished() {
  {
    QMutexLocker locker(&search_mutex_);
    Q_UNUSED(locker);

    providers_left_--;

    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    if(reply->error() == QNetworkReply::NoError) {
      CoverProvider* provider = providers_.take(reply);

      CoverSearchResults partial_results = provider->ParseReply(reply);
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
    if(providers_left_) {
      return;
    }
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
