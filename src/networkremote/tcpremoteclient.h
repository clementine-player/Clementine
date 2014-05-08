#ifndef TCPREMOTECLIENT_H
#define TCPREMOTECLIENT_H

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QBuffer>

#include "core/application.h"
#include "networkremote/remoteclient.h"
#include "remotecontrolmessages.pb.h"

class TcpRemoteClient : public RemoteClient {
  Q_OBJECT
 public:
  TcpRemoteClient(
      Application* app, QTcpSocket* client, QObject* parent = nullptr);
  ~TcpRemoteClient();

  // This method checks if client is authenticated before sending the data
  // from RemoteClient
  void SendData(pb::remote::Message* msg);
  void DisconnectClient(pb::remote::ReasonDisconnect reason);
  QAbstractSocket::SocketState state();

 private slots:
  void IncomingData();

signals:
  void Parse(const pb::remote::Message& msg);

 private:
  void ParseMessage(const QByteArray& data);

  // Sends data to client without check if authenticated
  void SendDataToClient(pb::remote::Message* msg);

  Application* app_;

  bool use_auth_code_;
  int auth_code_;
  bool authenticated_;
  bool allow_downloads_;
  bool downloader_;

  QTcpSocket* client_;
  bool reading_protobuf_;
  quint32 expected_length_;
  QBuffer buffer_;
};

#endif  // TCPREMOTECLIENT_H
