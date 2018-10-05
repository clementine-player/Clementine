/* This file is part of Clementine.
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2011, David Sansome <me@davidsansome.com>
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

#include "albumcoverfetchersearch.h"

#include <algorithm>
#include <cmath>

#include <QMutexLocker>
#include <QNetworkReply>
#include <QTimer>
#include <QtDebug>

#include "albumcoverfetcher.h"
#include "coverprovider.h"
#include "coverproviders.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

const int AlbumCoverFetcherSearch::kSearchTimeoutMs = 10000;
const int AlbumCoverFetcherSearch::kImageLoadTimeoutMs = 2500;
const int AlbumCoverFetcherSearch::kTargetSize = 500;
const float AlbumCoverFetcherSearch::kGoodScore = 1.85;

AlbumCoverFetcherSearch::AlbumCoverFetcherSearch(
    const CoverSearchRequest& request, QNetworkAccessManager* network,
    QObject* parent)
    : QObject(parent),
      request_(request),
      image_load_timeout_(new NetworkTimeouts(kImageLoadTimeoutMs, this)),
      network_(network),
      cancel_requested_(false) {
  // we will terminate the search after kSearchTimeoutMs miliseconds if we are
  // not
  // able to find all of the results before that point in time
  QTimer::singleShot(kSearchTimeoutMs, this, SLOT(TerminateSearch()));
}

void AlbumCoverFetcherSearch::TerminateSearch() {
  for (int id : pending_requests_.keys()) {
    pending_requests_.take(id)->CancelSearch(id);
  }

  AllProvidersFinished();
}

void AlbumCoverFetcherSearch::Start(CoverProviders* cover_providers) {
  for (CoverProvider* provider : cover_providers->List()) {
    connect(provider, SIGNAL(SearchFinished(int, QList<CoverSearchResult>)),
            SLOT(ProviderSearchFinished(int, QList<CoverSearchResult>)));
    const int id = cover_providers->NextId();
    const bool success =
        provider->StartSearch(request_.artist, request_.album, id);

    if (success) {
      pending_requests_[id] = provider;
      statistics_.network_requests_made_++;
    }
  }

  // end this search before it even began if there are no providers...
  if (pending_requests_.isEmpty()) {
    TerminateSearch();
  }
}

static bool CompareProviders(const CoverSearchResult& a,
                             const CoverSearchResult& b) {
  return a.provider < b.provider;
}

void AlbumCoverFetcherSearch::ProviderSearchFinished(
    int id, const QList<CoverSearchResult>& results) {
  if (!pending_requests_.contains(id)) return;

  CoverProvider* provider = pending_requests_.take(id);

  CoverSearchResults results_copy(results);
  // Set categories on the results
  for (int i = 0; i < results_copy.count(); ++i) {
    results_copy[i].provider = provider->name();
  }

  // Add results from the current provider to our pool
  results_.append(results_copy);
  statistics_.total_images_by_provider_[provider->name()]++;

  // do we have more providers left?
  if (!pending_requests_.isEmpty()) {
    return;
  }

  AllProvidersFinished();
}

void AlbumCoverFetcherSearch::AllProvidersFinished() {
  if (cancel_requested_) {
    return;
  }

  // if we only wanted to do the search then we're done
  if (request_.search) {
    emit SearchFinished(request_.id, results_);
    return;
  }

  // no results?
  if (results_.isEmpty()) {
    statistics_.missing_images_++;
    emit AlbumCoverFetched(request_.id, QImage());
    return;
  }

  // Now we have to load some images and figure out which one is the best.
  // We'll sort the list of results by category, then load the first few images
  // from each category and use some heuristics to score them.  If no images
  // are good enough we'll keep loading more images until we find one that is
  // or we run out of results.
  std::stable_sort(results_.begin(), results_.end(), CompareProviders);
  FetchMoreImages();
}

void AlbumCoverFetcherSearch::FetchMoreImages() {
  // Try the first one in each category.
  QString last_provider;
  for (int i = 0; i < results_.count(); ++i) {
    if (results_[i].provider == last_provider) {
      continue;
    }

    CoverSearchResult result = results_.takeAt(i--);
    last_provider = result.provider;

    qLog(Debug) << "Loading" << result.image_url << "from" << result.provider;

    RedirectFollower* image_reply =
        new RedirectFollower(network_->get(QNetworkRequest(result.image_url)));
    NewClosure(image_reply, SIGNAL(finished()), this,
               SLOT(ProviderCoverFetchFinished(RedirectFollower*)),
               image_reply);
    pending_image_loads_[image_reply] = result.provider;
    image_load_timeout_->AddReply(image_reply);

    statistics_.network_requests_made_++;
  }

  if (pending_image_loads_.isEmpty()) {
    // There were no more results?  Time to give up.
    SendBestImage();
  }
}

void AlbumCoverFetcherSearch::ProviderCoverFetchFinished(
    RedirectFollower* reply) {
  reply->deleteLater();
  const QString provider = pending_image_loads_.take(reply);

  statistics_.bytes_transferred_ += reply->bytesAvailable();

  if (cancel_requested_) {
    return;
  }

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Info) << "Error requesting" << reply->url() << reply->errorString();
  } else {
    QImage image;
    if (!image.loadFromData(reply->readAll())) {
      qLog(Info) << "Error decoding image data from" << reply->url();
    } else {
      const float score = ScoreImage(image);
      candidate_images_.insertMulti(score, CandidateImage(provider, image));

      qLog(Debug) << reply->url() << "scored" << score;
    }
  }

  if (pending_image_loads_.isEmpty()) {
    // We've fetched everything we wanted to fetch for now, check if we have an
    // image that's good enough.
    float best_score = 0.0;

    if (!candidate_images_.isEmpty()) {
      best_score = candidate_images_.keys().last();
    }

    qLog(Debug) << "Best image so far has a score of" << best_score;
    if (best_score >= kGoodScore) {
      SendBestImage();
    } else {
      FetchMoreImages();
    }
  }
}

float AlbumCoverFetcherSearch::ScoreImage(const QImage& image) const {
  // Invalid images score nothing
  if (image.isNull()) {
    return 0.0;
  }

  // A 500x500px image scores 1.0, bigger scores higher
  const float size_score =
      std::sqrt(static_cast<float>(image.width() * image.height())) / kTargetSize;

  // A 1:1 image scores 1.0, anything else scores less
  const float aspect_score = 1.0 - static_cast<float>(image.height() - image.width()) /
                                       std::max(image.height(), image.width());

  return size_score + aspect_score;
}

void AlbumCoverFetcherSearch::SendBestImage() {
  QImage image;

  if (!candidate_images_.isEmpty()) {
    const CandidateImage best_image = candidate_images_.values().back();
    image = best_image.second;

    statistics_.chosen_images_by_provider_[best_image.first]++;
    statistics_.chosen_images_++;
    statistics_.chosen_width_ += image.width();
    statistics_.chosen_height_ += image.height();
  } else {
    statistics_.missing_images_++;
  }

  emit AlbumCoverFetched(request_.id, image);
}

void AlbumCoverFetcherSearch::Cancel() {
  cancel_requested_ = true;

  if (!pending_requests_.isEmpty()) {
    TerminateSearch();
  } else if (!pending_image_loads_.isEmpty()) {
    for (RedirectFollower* reply : pending_image_loads_.keys()) {
      reply->abort();
    }
    pending_image_loads_.clear();
  }
}
