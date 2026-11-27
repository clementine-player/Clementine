#ifndef NETWORKREMOTE_H
#define NETWORKREMOTE_H

#include <QHostAddress>
#include <QImage>
#include <QList>
#include <QMetaObject>
#include <QNetworkAddressEntry>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QStringList>
#include <memory>
#include <vector>

#include "core/song.h"
#include "engines/enginebase.h"
#include "globalsearch/searchprovider.h"
#include "networkremote/protocolsniffer.h"
#include "playlist/playlistsequence.h"
#include "remotecontrolmessages.pb.h"

class Application;
class AuthAttemptLimiter;
class IncomingDataParser;
class InternetBrowser;
class Playlist;
class RemoteSmartPlaylists;
class MediaSync;
class MediaHttpServer;
class OutgoingDataCreator;
class RendererRegistry;
class QTcpServer;
class QTcpSocket;
class QTimer;
class RemoteClient;
class QMimeData;

// Bridges Application objects on the main thread to NetworkRemote's worker
// thread using queued signals and value snapshots.
class RemoteBridge : public QObject {
  Q_OBJECT
 public:
  explicit RemoteBridge(Application* app, QObject* parent = nullptr);
  ~RemoteBridge() override;

 public slots:
  void Init();
  void Shutdown();
  void SetHasClients(bool has_clients);
  void QueryClementineInfo(int client_id);
  void QueryFirstData(int client_id, bool send_playlist_songs);
  void QueryPlaylists(int client_id, bool include_closed);
  void QueryPlaylistSongs(int client_id, int playlist_id);
  void QueryTrackPosition();
  void StartGlobalSearch(int client_id, const QString& query);
  void QuerySavedRadios(int client_id);
  void HandleChangeSong(int playlist_id, int song_index, bool enqueue_mode);
  void SetCurrentPlaylistIfNotPlaying(int playlist_id);
  void PrepareDownloadSongs(int client_id,
                            const QByteArray& serialized_request);

 private slots:
  void OnActiveChanged(Playlist* playlist);
  void OnPlaylistChanged(Playlist* playlist);
  void FlushPlaylistUpdates();
  void OnPlaylistsListChanged();
  void OnArtLoaded(const Song& song, const QString& uri, const QImage& image);
  void OnGlobalSearchResults(int id, const SearchProvider::ResultList& results);
  void OnGlobalSearchFinished(int id);

 signals:
  void EngineStateChanged(Engine::State state);
  void VolumeChanged(int volume);
  void RepeatModeChanged(PlaylistSequence::RepeatMode mode);
  void ShuffleModeChanged(PlaylistSequence::ShuffleMode mode);
  void CurrentSongChanged(const Song& song, const QString& uri,
                          const QImage& image, int current_row,
                          int downloadability);
  void ClementineInfoStateReady(int client_id, Engine::State state);
  void FirstDataReady(int client_id, const Song& current_song, int current_row,
                      int current_downloadability, Engine::State state,
                      int volume, qint64 position_nanosec,
                      const QByteArray& playlists_pb,
                      const QByteArray& playlist_songs_pb,
                      PlaylistSequence::RepeatMode repeat_mode,
                      PlaylistSequence::ShuffleMode shuffle_mode);
  void SendRawToClient(int client_id, const QByteArray& data);
  void SendRawToAllClients(const QByteArray& data);
  void TrackPositionReady(qint64 position_nanosec);
  void PreparedDownloadSongs(int client_id,
                             const QByteArray& serialized_request,
                             const Song& current_song, int current_row,
                             const SongList& playlist_songs);

 private:
  struct GlobalSearchEntry {
    int client_id = 0;
    QString query;
  };

  QByteArray BuildPlaylistsProto(bool include_closed) const;
  QByteArray BuildPlaylistSongsProto(int id) const;
  int DownloadabilityOf(const Song& song) const;
  int ActiveCurrentRow() const;
  void DisconnectAll();

  QPointer<Application> app_;
  bool has_clients_;
  Song current_song_;
  QString current_uri_;
  QImage current_image_;
  QHash<int, GlobalSearchEntry> global_searches_;
  QSet<int> dirty_playlists_;
  QTimer* playlist_update_timer_;
  QList<QMetaObject::Connection> connections_;
};

class NetworkRemote : public QObject {
  Q_OBJECT
 public:
  static const char* kSettingsGroup;
  static const quint16 kDefaultServerPort;
  static const char* kTranscoderSettingPostfix;

  explicit NetworkRemote(Application* app, QObject* parent = nullptr);
  ~NetworkRemote();

  // What to listen on: every address when |all| is set, otherwise the
  // addresses in |present| that are in one of the |networks|, such as
  // "192.168.1.0/24", in the order of |present|. Networks that don't parse
  // are skipped.
  static QList<QHostAddress> ListenAddresses(
      bool all, const QStringList& networks,
      const QList<QHostAddress>& present);

  // This computer's addresses on the interfaces that are up, leaving out
  // IPv6 link-local ones: binding to those needs their scope as well, and
  // nobody types one into a phone anyway.
  static QList<QNetworkAddressEntry> PresentAddresses();

  // The network |address| is on, given its |prefix_length|, as text such as
  // "192.168.1.0/24".
  static QString NetworkOf(const QHostAddress& address, int prefix_length);

  // |text| as a network, such as "192.168.1.0/24", or an empty string when
  // it isn't one. A lone address is a network of just that address.
  static QString ParseNetwork(const QString& text);

  // The networks to listen on in place of the |addresses| saved by builds
  // that chose addresses rather than networks: the network each one is on
  // in |present|, or just that address when it isn't present.
  static QStringList NetworksOfAddresses(
      const QStringList& addresses, const QList<QNetworkAddressEntry>& present);

  // Whether a client at |address| counts as on the local network.
  static bool IpIsPrivate(const QHostAddress& address);

  // How the address a client connected to is being listened on.
  enum class Listening { OnAllAddresses, OnChosenAddress };

  // Whether a client at |peer| that connected to |local| counts as on the
  // local network. Besides the private ranges, when |local| is a listen
  // address the user chose and it's in a shared non-public range, such as
  // Tailscale's 100.64.0.0/10, clients from that same range count too.
  static bool IsLocalClient(const QHostAddress& peer, const QHostAddress& local,
                            Listening listening);

  // Whether any client counting as local could connect through a chosen
  // listen |address|. When not, only allowing local clients refuses everyone
  // on it.
  static bool LocalClientsCanReach(const QHostAddress& address);

  // What remotes call this computer: its host name, unless |name| (from
  // --remote-name) isn't empty. Set before the server starts. Not called
  // SetComputerName, which <windows.h> defines as a macro.
  static void SetRemoteName(const QString& name);
  static QString RemoteName();

  // Whether remotes can keep songs on the phone (from --media-sync). Off
  // while it's being built. Set before the server starts.
  static void SetMediaSyncEnabled(bool enabled);
  static bool MediaSyncEnabled();

  // A DISCONNECT message giving |reason|, framed as on the wire.
  static QByteArray DisconnectMessage(cpb::remote::ReasonDisconnect reason);

 signals:
  void AddToPlaylistSignal(QMimeData* data);
  void SetCurrentPlaylist(int id);
  void ClientDisconnected(int client_id);

 public slots:
  void SetupServer();
  void SetupMediaSync();
  void StartServer();
  void ReloadSettings();
  void AcceptConnection();
  void EnableKittens(bool aww);
  void SendKitten(quint64 id, const QImage& kitten);

 private slots:
  // Serialized messages from the RendererRegistry on the main thread.
  void SendToClient(int client_id, const QByteArray& data);
  void SendToAllClients(const QByteArray& data);
  void OnClientDisconnected(int client_id);
  void HandlePreparedDownload(int client_id,
                              const QByteArray& serialized_request,
                              const Song& current_song, int current_row,
                              const SongList& playlist_songs);
  // Listens on the chosen networks' addresses that have appeared since the
  // last time, such as after a new DHCP lease, and stops listening on the
  // ones that have gone.
  void Listen();

 private:
  // Lists and plays the library's smart playlists for remotes.
  void SetupSmartPlaylists();

  // One per active listening address. Sockets are reparented on accept so
  // closed servers can be erased safely.
  std::vector<std::unique_ptr<QTcpServer>> servers_;
  // Whether the server's started, listening on anything or not.
  bool running_;
  // The addresses being listened on now, and the ones that should be.
  QList<QHostAddress> listening_;
  QList<QHostAddress> wanted_;
  // While listening on chosen networks, checks for their addresses
  // changing.
  QTimer* address_check_;
  std::unique_ptr<IncomingDataParser> incoming_data_parser_;
  std::unique_ptr<OutgoingDataCreator> outgoing_data_creator_;
  // Bridges main-thread Application objects to the remote worker thread.
  QPointer<RemoteBridge> bridge_;
  // The Application's, from when the server first starts. It lives on the
  // main thread with the Player. Remotes only talk to it while streaming is
  // allowed.
  RendererRegistry* renderer_registry_;
  // Lives on the main thread with the Internet sidebar's model.
  InternetBrowser* internet_browser_;
  // Lives on the main thread with the library model.
  RemoteSmartPlaylists* smart_playlists_;
  // With --media-sync. Lives on the main thread, where open playlists are.
  MediaSync* media_sync_;
  // While the server runs, for Cast devices and renderers to fetch media.
  // Lives on this thread with the sockets.
  std::unique_ptr<MediaHttpServer> media_http_server_;

  quint16 port_;
  bool use_remote_;
  bool only_non_public_ip_;
  bool listen_on_all_addresses_;
  bool allow_streaming_;
  QStringList listen_networks_;
  Application* app_;

  QList<RemoteClient*> clients_;
  // Wrong auth codes, by address. Kept across connections and restarts of the
  // server, so neither resets the count.
  std::unique_ptr<AuthAttemptLimiter> auth_limiter_;

  void StopServer();
  void ReadSettings();
  void StartStreaming();
  void StopStreaming();
  // Tells the registry how media is being served, now that the server's
  // started, listening on |listening|, or stopped.
  void PublishServing(const QList<QHostAddress>& listening);
  // Listens on what the settings choose that isn't being listened on yet,
  // and closes what they no longer choose. Returns whether that changed
  // what's being listened on.
  bool Bind();
  // Tells the registry and Zeroconf what's being listened on.
  void Advertise();
  // Waits for a new connection's first byte to tell its protocol, then hands
  // it to a RemoteClient or the MediaHttpServer.
  void SniffProtocol(QTcpSocket* socket);
  // Tells a client that isn't on the local network why it's refused, in the
  // |protocol| it speaks, then closes the connection.
  void RefuseNotLocal(QTcpSocket* socket, ProtocolSniffer::Protocol protocol);
  void CreateRemoteClient(QTcpSocket* client_socket);
};

#endif  // NETWORKREMOTE_H
