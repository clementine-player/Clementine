#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>

class Player;

class HttpServer : public QObject {
  Q_OBJECT
 public:
  HttpServer(Player* player, QObject* parent = 0);
  bool Listen(const QHostAddress& addr, quint16 port);

 private slots:
  void NewConnection();

 private:
  QTcpServer server_;

  Player* player_;
};

#endif
