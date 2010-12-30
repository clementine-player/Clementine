#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QNetworkAccessManager>
#include <QString>

class QTcpSocket;

class HttpConnection : public QObject {
  Q_OBJECT
 public:
  HttpConnection(QTcpSocket* sock);

 private slots:
  void ReadyRead();

 private:
  void DoRequest();
  bool ParseRequest(const QString& line);

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
};

#endif
