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

class NetworkAccessManager;

class AlbumCoverFetcher : public QObject {
  Q_OBJECT

 public:
  AlbumCoverFetcher(NetworkAccessManager* network, QObject* parent = 0);
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
  void AlbumGetInfoFinished();
  void AlbumCoverFetchFinished();
  void StartRequests();
  void AlbumSearchFinished();

 private:
  struct QueuedRequest {
    quint64 id;
    QString query;
    QString artist;
    QString album;
  };

  void AddRequest(const QueuedRequest req);

  QNetworkAccessManager* network_;
  quint64 next_id_;

  QQueue<QueuedRequest> queued_requests_;
  QMap<QNetworkReply*, quint64> active_requests_;

  QTimer* request_starter_;
};

#endif  // ALBUMCOVERFETCHER_H
