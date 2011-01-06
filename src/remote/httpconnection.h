#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QNetworkAccessManager>
#include <QString>
#include <QVariantMap>

#include "core/song.h"

class QTcpSocket;

class AlbumCoverLoader;
class Player;

class HttpConnection : public QObject {
  Q_OBJECT
 public:
  HttpConnection(Player* player, QTcpSocket* sock);

 private slots:
  void ReadyRead();
  void ImageReady(quint64 id, const QImage& image);

 private:
  void DoRequest();
  bool ParseRequest(const QString& line);
  void SendHeaders();
  QVariantMap GetState();
  void SendSongInfo(Song song, const QImage& cover);

  QTcpSocket* socket_;

  enum State {
    WaitingForRequest,
    WaitingForHeaders,
    WaitingForRequestFinish,
    Finished,
    Error,
  } state_;

  QNetworkAccessManager::Operation method_;
  QString path_;

  Player* player_;
  AlbumCoverLoader* cover_loader_;

  QMap<quint64, Song> pending_tasks_;
};

#endif
