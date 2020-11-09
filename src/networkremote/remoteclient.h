#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QAbstractSocket>
#include <QBuffer>
#include <QTcpSocket>

#include "core/application.h"
#include "remotecontrolmessages.pb.h"
#include "songsender.h"

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

  SongSender* song_sender() { return song_sender_; }
  bool allow_downloads() const { return allow_downloads_; }

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
  SongSender* song_sender_;
};

#endif  // REMOTECLIENT_H
