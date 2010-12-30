#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>

class HttpServer : public QObject {
  Q_OBJECT
 public:
  HttpServer(QObject* parent = 0);
  bool Listen(const QHostAddress& addr, quint16 port);

 private slots:
  void NewConnection();
  void ReadyRead();

 private:
  QTcpServer server_;
};

#endif
