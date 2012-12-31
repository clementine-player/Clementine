#ifndef NETWORKREMOTE_H
#define NETWORKREMOTE_H

#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>

#include "core/player.h"
#include "core/application.h"
#include "incomingxmlparser.h"
#include "outgoingxmlcreator.h"

class NetworkRemote : public QThread {
    Q_OBJECT
public:
  static const char* kSettingsGroup;
  static const int kDefaultServerPort;

  NetworkRemote(Application* app);
  ~NetworkRemote();

public slots:
  void SetupServer();
  void StartServer();
  void ReloadSettings();
  void AcceptConnection();
  void IncomingData();

private:
  QTcpServer* server_;
  QList<QTcpSocket*>* clients_;
  IncomingXmlParser* incoming_xml_parser_;
  OutgoingXmlCreator* outgoing_xml_creator_;
  int port_;
  bool use_remote_;
  Application* app_;

  void StopServer();
  void ReadSettings();
};

#endif // NETWORKREMOTE_H
