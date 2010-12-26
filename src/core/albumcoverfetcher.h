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

#include <QImage>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>

#include <lastfm/Album>

#include <boost/scoped_ptr.hpp>

class QNetworkReply;
class QString;

// This class searches for album covers for a given query or artist/album and
// returns URLs.
class AlbumCoverFetcher : public QObject {
  Q_OBJECT

 public:
  AlbumCoverFetcher(QObject* parent = 0, QNetworkAccessManager* network = 0);
  virtual ~AlbumCoverFetcher() {}

  struct SearchResult {
    QString artist;
    QString album;
    QString image_url;
  };
  typedef QList<SearchResult> SearchResults;

  static const int kMaxConcurrentRequests;

  quint64 SearchForCovers(const QString& query);
  quint64 FetchAlbumCover(const QString& artist, const QString& album);

  void Clear();

 signals:
  void AlbumCoverFetched(quint64, const QImage& cover);
  void SearchFinished(quint64, const AlbumCoverFetcher::SearchResults& results);

 private slots:
  void AlbumSearchFinished();
  void AlbumCoverFetchFinished();
  void StartRequests();

 private:
  struct QueuedRequest {
    quint64 id;
    QString query;
    bool search;
  };

  void AddRequest(const QueuedRequest req);

  QNetworkAccessManager* network_;
  quint64 next_id_;

  QQueue<QueuedRequest> queued_requests_;
  QMap<QNetworkReply*, QueuedRequest> active_requests_;

  QTimer* request_starter_;
};

#endif  // ALBUMCOVERFETCHER_H
