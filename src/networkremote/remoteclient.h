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
  void Parse(const QByteArray& pb_data);

private:
  Application* app_;

  QTcpSocket* client_;
  bool reading_protobuf_;
  quint32 expected_length_;
  QBuffer buffer_;
};

#endif // REMOTECLIENT_H
