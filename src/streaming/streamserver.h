#ifndef STREAMSERVER_H
#define STREAMSERVER_H

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
  static const char* kSettingsGroup;
  static const quint16 kDefaultServerPort;

  StreamServer(Player* player, QObject* parent = nullptr);
  void Listen();
  void StopListening();

 private slots:
  void AcceptConnection();
  void ReadyRead(QTcpSocket* socket, QByteArray buffer);
  void AsyncLoadComplete(const UrlHandler::LoadResult& result);

 private:
  QByteArray ParseRequest(const QByteArray& data);
  void SendStream(const QUrl& url, QTcpSocket* socket);
  double getQuality();

  Player* player_;
  QTcpServer* server_;
  QMap<QUrl, QTcpSocket*> sockets_;
};

#endif  // STREAMSERVER_H
