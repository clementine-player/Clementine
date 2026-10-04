#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QHostAddress>
#include <QTcpSocket>
#include <atomic>

#include "core/application.h"
#include "remotecontrolmessages.pb.h"
#include "songsender.h"

class AuthAttemptLimiter;

class RemoteClient : public QObject {
  Q_OBJECT
 public:
  // |auth_limiter| is shared by every connection, and outlives this one.
  RemoteClient(Application* app, QTcpSocket* client,
               AuthAttemptLimiter* auth_limiter);
  ~RemoteClient();

  // This method checks if client is authenticated before sending the data
  void SendData(cpb::remote::Message* msg);
  QAbstractSocket::SocketState State();
  void setDownloader(bool downloader);
  bool isDownloader() { return downloader_; }
  // |retry_after_msec| is sent, rounded up to seconds, if it's set.
  void DisconnectClient(cpb::remote::ReasonDisconnect reason,
                        qint64 retry_after_msec = 0);

  SongSender* song_sender() { return song_sender_; }
  const QString& files_root_folder() const { return files_root_folder_; }
  const QStringList& files_music_extensions() const {
    return files_music_extensions_;
  }
  bool allow_downloads() const { return allow_downloads_; }

  // Unique for the life of the process, so other threads can refer to a
  // client without holding a pointer to it.
  int id() const { return id_; }
  QHostAddress local_address() const { return client_->localAddress(); }
  quint16 local_port() const { return client_->localPort(); }
  QHostAddress peer_address() const { return client_->peerAddress(); }

 private slots:
  void IncomingData();
  void SocketDisconnected();

 signals:
  void Parse(const cpb::remote::Message& msg);
  void Disconnected(int id);

 private:
  void ParseMessage(const QByteArray& data);

  // Sends data to client without check if authenticated
  void SendDataToClient(cpb::remote::Message* msg);

  static std::atomic<int> sNextId;

  Application* app_;
  const int id_;

  AuthAttemptLimiter* auth_limiter_;
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
  QStringList files_music_extensions_;
};

#endif  // REMOTECLIENT_H
