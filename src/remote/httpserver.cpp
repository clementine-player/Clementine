#include "httpserver.h"

#include <QTcpSocket>

#include "httpconnection.h"

HttpServer::HttpServer(QObject* parent)
    : QObject(parent) {
  connect(&server_, SIGNAL(newConnection()), SLOT(NewConnection()));
}

bool HttpServer::Listen(const QHostAddress& addr, quint16 port) {
  return server_.listen(addr, port);
}

void HttpServer::NewConnection() {
  qDebug() << Q_FUNC_INFO;
  QTcpSocket* socket = server_.nextPendingConnection();
  HttpConnection* conn = new HttpConnection(socket);
}

void HttpServer::ReadyRead() {
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  Q_ASSERT(socket);
  while (socket->canReadLine()) {
    QByteArray line = socket->readLine();
  }
}
