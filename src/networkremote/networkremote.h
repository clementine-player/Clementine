#ifndef NETWORKREMOTE_H
#define NETWORKREMOTE_H

#include <QList>
#include <QObject>
#include <QStringList>
#include <memory>
#include <vector>

class Application;
class IncomingDataParser;
class OutgoingDataCreator;
class QHostAddress;
class QImage;
class QTcpServer;
class QTcpSocket;
class RemoteClient;
class QMimeData;

class NetworkRemote : public QObject {
  Q_OBJECT
 public:
  static const char* kSettingsGroup;
  static const quint16 kDefaultServerPort;
  static const char* kTranscoderSettingPostfix;

  explicit NetworkRemote(Application* app, QObject* parent = nullptr);
  ~NetworkRemote();

  // What to listen on: every address when |all| is set, otherwise the
  // addresses in |chosen| that parse, in the order given.
  static QList<QHostAddress> ListenAddresses(bool all,
                                             const QStringList& chosen);

 signals:
  void AddToPlaylistSignal(QMimeData* data);
  void SetCurrentPlaylist(int id);

 public slots:
  void SetupServer();
  void StartServer();
  void ReloadSettings();
  void AcceptConnection();
  void EnableKittens(bool aww);
  void SendKitten(quint64 id, const QImage& kitten);

 private:
  // One per address being listened on.
  std::vector<std::unique_ptr<QTcpServer>> servers_;
  std::unique_ptr<IncomingDataParser> incoming_data_parser_;
  std::unique_ptr<OutgoingDataCreator> outgoing_data_creator_;

  quint16 port_;
  bool use_remote_;
  bool only_non_public_ip_;
  bool listen_on_all_addresses_;
  QStringList listen_addresses_;
  bool signals_connected_;
  Application* app_;

  QList<RemoteClient*> clients_;

  void StopServer();
  void ReadSettings();
  void CreateRemoteClient(QTcpSocket* client_socket);
  bool IpIsPrivate(const QHostAddress& address);
};

#endif  // NETWORKREMOTE_H
