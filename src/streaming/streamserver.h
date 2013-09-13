#ifndef STREAMSERVER_H
#define STREAMSERVER_h

#include <QByteArray>
#include <QObject>
#include <QMap>
#include <QUrl>

#include "core/urlhandler.h"

class Player;
class QTcpServer;
class QTcpSocket;

class StreamServer : public QObject {
  Q_OBJECT
 public:
  StreamServer(Player* player, QObject* parent = 0);
  void Listen(quint16 port = 8080);

 private slots:
  void AcceptConnection();
  void ReadyRead(QTcpSocket* socket, QByteArray buffer);
  void AsyncLoadComplete(const UrlHandler::LoadResult& result);

 private:
  QByteArray ParseRequest(const QByteArray& data);
  void SendStream(const QUrl& url, QTcpSocket* socket);

  Player* player_;
  QTcpServer* server_;
  QMap<QUrl, QTcpSocket*> sockets_;
};

#endif  // STREAMSERVER_H
