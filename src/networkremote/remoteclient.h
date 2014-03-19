#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QBuffer>

#include "core/application.h"
#include "remotecontrolmessages.pb.h"

class RemoteClient : public QObject {
  Q_OBJECT
 public:
  RemoteClient(Application* app, QTcpSocket* client);
  ~RemoteClient();

  // This method checks if client is authenticated before sending the data
  void SendData(pb::remote::Message* msg);
  QAbstractSocket::SocketState State();
  void setDownloader(bool downloader);
  bool isDownloader() { return downloader_; }
  void DisconnectClient(pb::remote::ReasonDisconnect reason);

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

#endif  // REMOTECLIENT_H
