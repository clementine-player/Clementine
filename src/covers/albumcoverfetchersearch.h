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

#ifndef ALBUMCOVERFETCHERSEARCH_H
#define ALBUMCOVERFETCHERSEARCH_H

#include "albumcoverfetcher.h"

#include <QMap>
#include <QObject>

class CoverProvider;
class QNetworkAccessManager;
class QNetworkReply;

// This class encapsulates a single search for covers initiated by an
// AlbumCoverFetcher. The search engages all of the known cover providers.
// AlbumCoverFetcherSearch signals search results to an interested
// AlbumCoverFetcher when all of the providers have done their part.
class AlbumCoverFetcherSearch : public QObject {
  Q_OBJECT

 public:
  // A timeout (in miliseconds) for every search.
  static const int kSearchTimeout;

  AlbumCoverFetcherSearch(const CoverSearchRequest& request,
                          QNetworkAccessManager* network, QObject* parent);

  // Starts the search. This is the moment when we count cover providers available
  // in the application.
  void Start();

signals:
  // It's the end of search (when there was no fetch-me-a-cover request).
  void SearchFinished(quint64, const CoverSearchResults& results);
  // It's the end of search and we've fetched a cover.
  void AlbumCoverFetched(quint64, const QImage& cover);

private slots:
  void ProviderSearchFinished(int id, const QList<CoverSearchResult>& results);
  void ProviderCoverFetchFinished();
  void Timeout();

private:
  // Timeouts this search.
  void TerminateSearch();

  // Search request encapsulated by this AlbumCoverFetcherSearch.
  CoverSearchRequest request_;
  // Complete results (from all of the available providers).
  CoverSearchResults results_;

  QMap<int, CoverProvider*> pending_requests_;

  QNetworkAccessManager* network_;
};

#endif  // ALBUMCOVERFETCHERSEARCH_H
