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

#ifndef COVERS_ALBUMCOVERFETCHERSEARCH_H_
#define COVERS_ALBUMCOVERFETCHERSEARCH_H_

#include "albumcoverfetcher.h"

#include <QMap>
#include <QObject>

class CoverProvider;
class CoverProviders;
class NetworkTimeouts;
class NetworkAccessManager;
class RedirectFollower;

// This class encapsulates a single search for covers initiated by an
// AlbumCoverFetcher. The search engages all of the known cover providers.
// AlbumCoverFetcherSearch signals search results to an interested
// AlbumCoverFetcher when all of the providers have done their part.
class AlbumCoverFetcherSearch : public QObject {
  Q_OBJECT

 public:
  AlbumCoverFetcherSearch(const CoverSearchRequest& request,
                          QNetworkAccessManager* network, QObject* parent);

  void Start(CoverProviders* cover_providers);

  // Cancels all pending requests.  No Finished signals will be emitted, and it
  // is the caller's responsibility to delete the AlbumCoverFetcherSearch.
  void Cancel();

  CoverSearchStatistics statistics() const { return statistics_; }

 signals:
  // It's the end of search (when there was no fetch-me-a-cover request).
  void SearchFinished(quint64, const CoverSearchResults& results);

  // It's the end of search and we've fetched a cover.
  void AlbumCoverFetched(quint64, const QImage& cover);

 private slots:
  void ProviderSearchFinished(int id, const QList<CoverSearchResult>& results);
  void ProviderCoverFetchFinished(RedirectFollower* reply);
  void TerminateSearch();

 private:
  void AllProvidersFinished();

  void FetchMoreImages();
  float ScoreImage(const QImage& image) const;
  void SendBestImage();

 private:
  static const int kSearchTimeoutMs;
  static const int kImageLoadTimeoutMs;
  static const int kTargetSize;
  static const float kGoodScore;

  CoverSearchStatistics statistics_;

  // Search request encapsulated by this AlbumCoverFetcherSearch.
  CoverSearchRequest request_;

  // Complete results (from all of the available providers).
  CoverSearchResults results_;

  QMap<int, CoverProvider*> pending_requests_;
  QMap<RedirectFollower*, QString> pending_image_loads_;
  NetworkTimeouts* image_load_timeout_;

  // QMap is sorted by key (score).  Values are (provider_name, image)
  typedef QPair<QString, QImage> CandidateImage;
  QMap<float, CandidateImage> candidate_images_;

  QNetworkAccessManager* network_;

  bool cancel_requested_;
};

#endif  // COVERS_ALBUMCOVERFETCHERSEARCH_H_
