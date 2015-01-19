#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QAbstractSocket>
#include <QObject>

#include "songsender.h"

#include "core/application.h"
#include "remotecontrolmessages.pb.h"

class Application;
class QByteArray;

class RemoteClient : public QObject {
  Q_OBJECT
 public:
  RemoteClient(Application* app, QObject* parent = nullptr);

  virtual void SendData(pb::remote::Message* msg) = 0;
  virtual void DisconnectClient(pb::remote::ReasonDisconnect reason) = 0;
  virtual QAbstractSocket::SocketState state() = 0;

  void set_downloader(bool downloader) { downloader_ = downloader; }
  bool downloader() const { return downloader_; }
  SongSender* song_sender() { return song_sender_; }

 signals:
  void Parse(const pb::remote::Message& msg);

 protected:
  Application* app_;
  bool downloader_;
};

#endif  // REMOTECLIENT_H
