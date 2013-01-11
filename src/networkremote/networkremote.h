#ifndef NETWORKREMOTE_H
#define NETWORKREMOTE_H

#include <QTcpServer>
#include <QTcpSocket>

#include "core/player.h"
#include "core/application.h"
#include "incomingdataparser.h"
#include "outgoingdatacreator.h"
#include "remoteclient.h"

class NetworkRemote : public QThread {
    Q_OBJECT
public:
  static const char* kSettingsGroup;
  static const int kDefaultServerPort;
  static const int kProtocolBufferVersion;

  NetworkRemote(Application* app);
  ~NetworkRemote();

public slots:
  void SetupServer();
  void StartServer();
  void ReloadSettings();
  void AcceptConnection();

private:
  QTcpServer* server_;
  QTcpServer* server_ipv6_;
  IncomingDataParser* incoming_data_parser_;
  OutgoingDataCreator* outgoing_data_creator_;
  int port_;
  bool use_remote_;
  bool only_non_public_ip_;
  bool signals_connected_;
  Application* app_;

  QList<RemoteClient*> clients_;

  void StopServer();
  void ReadSettings();
  void CreateRemoteClient(QTcpSocket* client_socket);
  bool IpIsPrivate(int ip);
};

#endif // NETWORKREMOTE_H
