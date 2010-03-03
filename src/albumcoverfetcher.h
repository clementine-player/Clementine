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

class AlbumCoverFetcher : public QObject {
  Q_OBJECT

 public:
  AlbumCoverFetcher(QObject* parent = 0, QNetworkAccessManager* network_ = 0);
  virtual ~AlbumCoverFetcher() {}

  static const int kMaxConcurrentRequests;

  quint64 FetchAlbumCover(const QString& artist, const QString& album);

  void Clear();

 signals:
  void AlbumCoverFetched(quint64, const QImage& cover);

 private slots:
  void AlbumGetInfoFinished();
  void AlbumCoverFetchFinished();
  void StartRequests();

 private:
  struct QueuedRequest {
    quint64 id;
    QString artist;
    QString album;
  };

  boost::scoped_ptr<QNetworkAccessManager> network_;
  quint64 next_id_;

  QQueue<QueuedRequest> queued_requests_;
  QMap<QNetworkReply*, quint64> active_requests_;

  QTimer* request_starter_;
};

#endif  // ALBUMCOVERFETCHER_H
