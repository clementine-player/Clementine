#include "httpserver.h"

#include <QTcpSocket>
#include <QtDebug>

#include "httpconnection.h"

HttpServer::HttpServer(Player* player, QObject* parent)
    : QObject(parent),
      player_(player) {
  connect(&server_, SIGNAL(newConnection()), SLOT(NewConnection()));
}

bool HttpServer::Listen(const QHostAddress& addr, quint16 port) {
  return server_.listen(addr, port);
}

void HttpServer::NewConnection() {
  qDebug() << Q_FUNC_INFO;
  QTcpSocket* socket = server_.nextPendingConnection();
  new HttpConnection(player_, socket);
}
