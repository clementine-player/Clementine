#ifndef ALBUMCOVERFETCHER_H
#define ALBUMCOVERFETCHER_H

#include <QImage>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>

#include <lastfm/Album>

class QNetworkReply;
class QString;

class AlbumCoverFetcher : public QObject {
  Q_OBJECT

 public:
  AlbumCoverFetcher(QObject* parent = 0);
  virtual ~AlbumCoverFetcher() {}

  lastfm::Album FetchAlbumCover(const QString& artist, const QString& album);

 signals:
  void AlbumCoverFetched(const lastfm::Album&, const QImage& cover);

 private slots:
  void AlbumGetInfoFinished();
  void AlbumCoverFetchFinished();

 private:
  QNetworkAccessManager network_;
  QMap<QNetworkReply*, lastfm::Album> requests_;
};

#endif  // ALBUMCOVERFETCHER_H
