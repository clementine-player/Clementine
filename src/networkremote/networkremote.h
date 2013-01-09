#ifndef NETWORKREMOTE_H
#define NETWORKREMOTE_H

#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>

#include "core/player.h"
#include "core/application.h"
#include "incomingdataparser.h"
#include "outgoingdatacreator.h"

class Application;

class NetworkRemote : public QThread {
    Q_OBJECT
public:
  static const char* kSettingsGroup;
  static const int kDefaultServerPort;

  NetworkRemote(Application* app);
  ~NetworkRemote();

  void RemoveClient(int index);

public slots:
  void SetupServer();
  void StartServer();
  void ReloadSettings();
  void AcceptConnection();
  void IncomingData();

private:
  QTcpServer* server_;
  IncomingDataParser* incoming_data_parser_;
  OutgoingDataCreator* outgoing_data_creator_;
  int port_;
  bool use_remote_;
  bool signals_connected_;
  Application* app_;

  QList<QTcpSocket*> clients_;
  QList<bool> reading_protobuf_;
  QList<quint32> expected_length_;
  QList<QBuffer*> buffer_;

  void StopServer();
  void ReadSettings();
};

#endif // NETWORKREMOTE_H
