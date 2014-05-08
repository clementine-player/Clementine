#ifndef NETWORKREMOTE_H
#define NETWORKREMOTE_H

#include <memory>

#include <QObject>

class Application;
class ClementineWebPage;
class IncomingDataParser;
class OutgoingDataCreator;
class QHostAddress;
class QImage;
class QTcpServer;
class QTcpSocket;
class RemoteClient;

class NetworkRemote : public QObject {
  Q_OBJECT
 public:
  static const char* kSettingsGroup;
  static const quint16 kDefaultServerPort;

  explicit NetworkRemote(
      ClementineWebPage* web_channel, Application* app, QObject* parent = nullptr);
  ~NetworkRemote();

 public slots:
  void SetupServer();
  void StartServer();
  void ReloadSettings();
  void AcceptConnection();
  void EnableKittens(bool aww);
  void SendKitten(quint64 id, const QImage& kitten);

 private slots:
  void AcceptWebConnection();

 private:
  void ConnectSignals();

  std::unique_ptr<QTcpServer> server_;
  std::unique_ptr<QTcpServer> server_ipv6_;
  std::unique_ptr<IncomingDataParser> incoming_data_parser_;
  std::unique_ptr<OutgoingDataCreator> outgoing_data_creator_;
  std::unique_ptr<ClementineWebPage> web_channel_;

  quint16 port_;
  bool use_remote_;
  bool only_non_public_ip_;
  bool signals_connected_;
  Application* app_;

  QList<RemoteClient*> clients_;

  void StopServer();
  void ReadSettings();
  void CreateRemoteClient(QTcpSocket* client_socket);
  bool IpIsPrivate(const QHostAddress& address);
};

#endif  // NETWORKREMOTE_H
