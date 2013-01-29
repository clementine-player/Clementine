#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QBuffer>

#include "core/application.h"
#include "incomingdataparser.h"

class RemoteClient : public QObject {
    Q_OBJECT
public:
  RemoteClient(Application* app, QTcpSocket* client);
  ~RemoteClient();

  void SendData(pb::remote::Message* msg);
  QAbstractSocket::SocketState State();

private slots:
  void IncomingData();

signals:
  void Parse(const pb::remote::Message& msg);

private:
  void ParseMessage(const QByteArray& data);
  void DisconnectClientWrongAuthCode();

  Application* app_;

  bool use_auth_code_;
  int auth_code_;

  QTcpSocket* client_;
  bool reading_protobuf_;
  quint32 expected_length_;
  QBuffer buffer_;
};

#endif // REMOTECLIENT_H
