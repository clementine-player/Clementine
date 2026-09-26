#ifndef NETWORKREMOTE_H
#define NETWORKREMOTE_H

#include <QList>
#include <QObject>
#include <QStringList>
#include <memory>
#include <vector>

class Application;
class IncomingDataParser;
class MediaHttpServer;
class OutgoingDataCreator;
class RendererRegistry;
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

  // Whether a client at |address| counts as on the local network.
  static bool IpIsPrivate(const QHostAddress& address);

 signals:
  void AddToPlaylistSignal(QMimeData* data);
  void SetCurrentPlaylist(int id);
  void ClientDisconnected(int client_id);

 public slots:
  void SetupServer();
  void StartServer();
  void ReloadSettings();
  void AcceptConnection();
  void EnableKittens(bool aww);
  void SendKitten(quint64 id, const QImage& kitten);

 private slots:
  // Serialized messages from the RendererRegistry on the main thread.
  void SendToClient(int client_id, const QByteArray& data);
  void SendToAllClients(const QByteArray& data);

 private:
  // One per address being listened on.
  std::vector<std::unique_ptr<QTcpServer>> servers_;
  std::unique_ptr<IncomingDataParser> incoming_data_parser_;
  std::unique_ptr<OutgoingDataCreator> outgoing_data_creator_;
  // Only while streaming is allowed. The registry lives on the main thread
  // with the Player; the HTTP server on this one with the sockets.
  RendererRegistry* renderer_registry_;
  std::unique_ptr<MediaHttpServer> media_http_server_;

  quint16 port_;
  bool use_remote_;
  bool only_non_public_ip_;
  bool listen_on_all_addresses_;
  bool allow_streaming_;
  QStringList listen_addresses_;
  bool signals_connected_;
  Application* app_;

  QList<RemoteClient*> clients_;

  void StopServer();
  void ReadSettings();
  void StartStreaming();
  void StopStreaming();
  // Waits for a new connection's first byte to tell its protocol, then hands
  // it to a RemoteClient or the MediaHttpServer.
  void SniffProtocol(QTcpSocket* socket);
  void CreateRemoteClient(QTcpSocket* client_socket);
};

#endif  // NETWORKREMOTE_H
