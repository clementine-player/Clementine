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

#ifndef ALBUMCOVERFETCHER_H
#define ALBUMCOVERFETCHER_H

#include "coversearchstatistics.h"

#include <QHash>
#include <QImage>
#include <QList>
#include <QMetaType>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>
#include <QUrl>

class QNetworkReply;
class QString;

class AlbumCoverFetcherSearch;
class CoverProviders;

// This class represents a single search-for-cover request. It identifies
// and describes the request.
struct CoverSearchRequest {
  // an unique (for one AlbumCoverFetcher) request identifier
  quint64 id;

  // a search query
  QString artist;
  QString album;

  // is this only a search request or should we also fetch the first
  // cover that's found?
  bool search;
};

// This structure represents a single result of some album's cover search
// request.
// It contains an URL that leads to a found cover plus its description (usually
// the "artist - album" string).
struct CoverSearchResult {
  // used for grouping in the user interface.  This is set automatically - don't
  // set it manually in your cover provider.
  QString provider;

  // description of this result (we suggest using the "artist - album" format)
  QString description;

  // an URL of a cover image described by this CoverSearchResult
  QUrl image_url;
};
Q_DECLARE_METATYPE(CoverSearchResult);

// This is a complete result of a single search request (a list of results, each
// describing one image, actually).
typedef QList<CoverSearchResult> CoverSearchResults;
Q_DECLARE_METATYPE(QList<CoverSearchResult>);

// This class searches for album covers for a given query or artist/album and
// returns URLs. It's NOT thread-safe.
class AlbumCoverFetcher : public QObject {
  Q_OBJECT

 public:
  AlbumCoverFetcher(CoverProviders* cover_providers, QObject* parent = nullptr,
                    QNetworkAccessManager* network = 0);
  virtual ~AlbumCoverFetcher() {}

  static const int kMaxConcurrentRequests;

  quint64 SearchForCovers(const QString& artist, const QString& album);
  quint64 FetchAlbumCover(const QString& artist, const QString& album);

  void Clear();

signals:
  void AlbumCoverFetched(quint64, const QImage& cover,
                         const CoverSearchStatistics& statistics);
  void SearchFinished(quint64, const CoverSearchResults& results,
                      const CoverSearchStatistics& statistics);

 private slots:
  void SingleSearchFinished(quint64, CoverSearchResults results);
  void SingleCoverFetched(quint64, const QImage& cover);
  void StartRequests();

 private:
  void AddRequest(const CoverSearchRequest& req);

  CoverProviders* cover_providers_;
  QNetworkAccessManager* network_;
  quint64 next_id_;

  QQueue<CoverSearchRequest> queued_requests_;
  QHash<quint64, AlbumCoverFetcherSearch*> active_requests_;

  QTimer* request_starter_;
};

#endif  // ALBUMCOVERFETCHER_H
