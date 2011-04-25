#ifndef SPOTIFYCLIENT_H
#define SPOTIFYCLIENT_H

#include <QObject>

class QTcpSocket;

class ResponseMessage;

class SpotifyClient : public QObject {
  Q_OBJECT

public:
  SpotifyClient(QObject* parent = 0);

  void Init(quint16 port);

  void LoginCompleted(bool success, const QString& error);

signals:
  void Login(const QString& username, const QString& password);

private slots:
  void SocketReadyRead();

private:
  void SendMessage(const ResponseMessage& message);

  QTcpSocket* socket_;
};

#endif // SPOTIFYCLIENT_H
