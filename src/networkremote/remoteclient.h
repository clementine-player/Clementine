#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QAbstractSocket>
#include <QObject>

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

 signals:
  void ParseMessage(const QByteArray& data);

 private:
  Application* app_;
  bool downloader_;
};

#endif  // REMOTECLIENT_H
