#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

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
  const QString& files_root_folder() const { return files_root_folder_; }
  const QString& music_extensions() const { return music_extensions_; }
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
  QByteArray buffer_;
  SongSender* song_sender_;

  QString files_root_folder_;
  QString music_extensions_;
};

#endif  // REMOTECLIENT_H
