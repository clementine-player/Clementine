#ifndef WEBREMOTECLIENT_H
#define WEBREMOTECLIENT_H

#include <memory>

#include "networkremote/remoteclient.h"

class ClementineWebPage;
class NetworkAccessManager;

class WebRemoteClient : public RemoteClient {
  Q_OBJECT
 public:
  WebRemoteClient(
      ClementineWebPage* web_channel, Application* app, QObject* parent = nullptr);
  ~WebRemoteClient();

  // RemoteClient
  void SendData(pb::remote::Message* msg);
  void DisconnectClient(pb::remote::ReasonDisconnect reason);
  QAbstractSocket::SocketState state();

 private:
  ClementineWebPage* web_channel_;
  std::unique_ptr<NetworkAccessManager> network_;
};

#endif  // WEBREMOTECLIENT_H
