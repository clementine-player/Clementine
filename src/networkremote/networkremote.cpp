/* This file is part of Clementine.
   Copyright 2012, Andreas Muttscheller <asfa194@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "networkremote.h"

#include <QDataStream>
#include <QHostInfo>
#include <QNetworkProxy>
#include <QSettings>
#include <QTcpServer>
#include <QTimer>

#include "chromecast/castdiscovery.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "covers/currentartloader.h"
#include "engines/enginerouter.h"
#include "internet/core/internetmodel.h"
#include "networkremote/authattemptlimiter.h"
#include "networkremote/incomingdataparser.h"
#include "networkremote/internetbrowser.h"
#include "networkremote/outgoingdatacreator.h"
#include "networkremote/protocolsniffer.h"
#include "networkremote/streaming/mediahttpserver.h"
#include "networkremote/streaming/rendererregistry.h"
#include "networkremote/zeroconf.h"
#include "playlist/playlistmanager.h"

const char* NetworkRemote::kSettingsGroup = "NetworkRemote";
const quint16 NetworkRemote::kDefaultServerPort = 5500;
const char* NetworkRemote::kTranscoderSettingPostfix = "/NetworkRemote";

namespace {
// How long a new connection may stay silent before it's dropped.
const int kFirstByteTimeoutMsec = 10000;
const char* kSniffedProperty = "clementine_sniffed";
// Set on a connection that isn't from the local network when only those are
// allowed. It's told why once it's said which protocol it speaks.
const char* kRefusedProperty = "clementine_refused";

// Non-public ranges that aren't a local network by themselves. Tailscale
// gives its devices addresses from carrier-grade NAT space, so a listen
// address chosen there makes the rest of it local. Only a chosen one: when
// listening on everything, 100.x clients stay public, as before.
const char* kSharedSubnets[] = {"100.64.0.0/10", "::ffff:100.64.0.0/106"};

// From --remote-name; empty for the host name.
QString sRemoteName;

bool InSharedSubnet(const QHostAddress& address) {
  for (const char* subnet : kSharedSubnets) {
    if (address.isInSubnet(QHostAddress::parseSubnet(subnet))) return true;
  }
  return false;
}
}  // namespace

NetworkRemote::NetworkRemote(Application* app, QObject* parent)
    : QObject(parent),
      renderer_registry_(nullptr),
      internet_browser_(nullptr),
      allow_streaming_(false),
      signals_connected_(false),
      app_(app),
      auth_limiter_(new AuthAttemptLimiter) {
  setObjectName("Network remote");
}

NetworkRemote::~NetworkRemote() {
  StopServer();
  if (internet_browser_) internet_browser_->deleteLater();
}

void NetworkRemote::ReadSettings() {
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);
  use_remote_ = s.value("use_remote", false).toBool();
  port_ = s.value("port", kDefaultServerPort).toInt();

  // Use only non public ips must be true be default
  only_non_public_ip_ = s.value("only_non_public_ip", true).toBool();

  listen_on_all_addresses_ = s.value("listen_on_all_addresses", true).toBool();
  listen_addresses_ = s.value("listen_addresses").toStringList();
  allow_streaming_ = s.value("allow_streaming", false).toBool();

  s.endGroup();
}

void NetworkRemote::SetRemoteName(const QString& name) {
  sRemoteName = name.trimmed();
}

QString NetworkRemote::RemoteName() {
  return sRemoteName.isEmpty() ? QHostInfo::localHostName() : sRemoteName;
}

QList<QHostAddress> NetworkRemote::ListenAddresses(bool all,
                                                   const QStringList& chosen) {
  if (all) {
    // Any is dual-stack, so it covers IPv6 too. The separate AnyIPv6 server
    // is left from Qt 4, where Any meant IPv4 only; on a dual-stack system
    // it fails to bind alongside Any, which is expected.
    return {QHostAddress(QHostAddress::Any),
            QHostAddress(QHostAddress::AnyIPv6)};
  }

  QList<QHostAddress> addresses;
  for (const QString& text : chosen) {
    QHostAddress address(text);
    if (address.isNull()) {
      qLog(Warning) << "Ignoring a listen address that doesn't parse:" << text;
      continue;
    }
    addresses << address;
  }
  return addresses;
}

void NetworkRemote::SetupServer() {
  incoming_data_parser_.reset(new IncomingDataParser(app_));
  outgoing_data_creator_.reset(new OutgoingDataCreator(app_));

  outgoing_data_creator_->SetClients(&clients_);

  connect(app_->current_art_loader(),
          SIGNAL(ArtLoaded(const Song&, const QString&, const QImage&)),
          outgoing_data_creator_.get(),
          SLOT(CurrentSongChanged(const Song&, const QString&, const QImage&)));

  connect(incoming_data_parser_.get(), SIGNAL(AddToPlaylistSignal(QMimeData*)),
          SIGNAL(AddToPlaylistSignal(QMimeData*)));
  connect(incoming_data_parser_.get(), SIGNAL(SetCurrentPlaylist(int)),
          SIGNAL(SetCurrentPlaylist(int)));

  // Browsing the Internet sidebar. The browser works on the sidebar's model,
  // so it lives on the main thread, and gets the model there.
  internet_browser_ = new InternetBrowser;
  internet_browser_->moveToThread(QCoreApplication::instance()->thread());
  Application* app = app_;
  InternetBrowser* browser = internet_browser_;
  QMetaObject::invokeMethod(
      internet_browser_,
      [app, browser]() {
        browser->SetModel(app->internet_model()->merged_model(),
                          InternetBrowser::HooksFor(app->internet_model()));
      },
      Qt::QueuedConnection);
  connect(incoming_data_parser_.get(), SIGNAL(BrowseMessage(int, QByteArray)),
          internet_browser_, SLOT(HandleMessage(int, QByteArray)));
  connect(this, SIGNAL(ClientDisconnected(int)), internet_browser_,
          SLOT(ClientDisconnected(int)));
  connect(internet_browser_, SIGNAL(SendToClient(int, QByteArray)), this,
          SLOT(SendToClient(int, QByteArray)));
  // As the network remote's other additions: MainWindow drops it on the
  // playlist.
  connect(internet_browser_, SIGNAL(AddToPlaylist(QMimeData*)), this,
          SIGNAL(AddToPlaylistSignal(QMimeData*)));
}

void NetworkRemote::StartServer() {
  if (!app_) {
    qLog(Error) << "Start Server called without having an application!";
    return;
  }
  // Check if user desires to start a network remote server
  ReadSettings();
  if (!use_remote_) {
    qLog(Info) << "Network Remote deactivated";
    return;
  }

  if (!servers_.empty()) {
    // Already running; ReloadSettings stops the servers before restarting.
    return;
  }

  qLog(Info) << "Starting network remote";

  const QList<QHostAddress> addresses =
      ListenAddresses(listen_on_all_addresses_, listen_addresses_);
  if (addresses.isEmpty()) {
    qLog(Warning) << "Network remote enabled, but no listen address is chosen";
  }

  StartStreaming();

  QList<QHostAddress> listening;
  for (const QHostAddress& address : addresses) {
    std::unique_ptr<QTcpServer> server(new QTcpServer);
    server->setProxy(QNetworkProxy::NoProxy);
    connect(server.get(), SIGNAL(newConnection()), this,
            SLOT(AcceptConnection()));

    if (server->listen(address, port_)) {
      qLog(Info) << "Listening on" << address.toString() << "port" << port_;
      listening << address;
    } else if (listen_on_all_addresses_) {
      // See ListenAddresses: one of the two wildcard servers failing is
      // normal.
      qLog(Debug) << "Couldn't listen on" << address.toString() << "port"
                  << port_ << ":" << server->errorString();
    } else {
      qLog(Warning) << "Couldn't listen on" << address.toString() << "port"
                    << port_ << ":" << server->errorString();
    }
    servers_.push_back(std::move(server));
  }

  if (renderer_registry_) {
    // The registry lives on the Player's thread.
    RendererRegistry* registry = renderer_registry_;
    const quint16 port = port_;
    const QList<QHostAddress> where =
        listen_on_all_addresses_ ? QList<QHostAddress>() : listening;
    QMetaObject::invokeMethod(
        registry,
        [registry, port, where]() { registry->SetListening(port, where); },
        Qt::QueuedConnection);
  }

  if (Zeroconf::GetZeroconf()) {
    // Advertise only where something is listening, so remotes aren't sent to
    // an address that refuses them. That includes a chosen address that
    // couldn't be bound, such as a VPN's while it's down.
    if (!listen_on_all_addresses_ && listening.isEmpty()) {
      qLog(Warning) << "Not listening on any address, so not advertising";
    } else {
      QString name = QString("Clementine on %1").arg(RemoteName());
      Zeroconf::GetZeroconf()->Publish(
          "local", "_clementine._tcp", name, port_,
          listen_on_all_addresses_ ? QList<QHostAddress>() : listening);
    }
  }
}

void NetworkRemote::StopServer() {
  if (servers_.empty()) return;

  if (Zeroconf::GetZeroconf()) {
    Zeroconf::GetZeroconf()->Unpublish();
  }

  if (outgoing_data_creator_) {
    outgoing_data_creator_->DisconnectAllClients();
  }
  // Deleting a server closes it.
  servers_.clear();
  qDeleteAll(clients_);
  clients_.clear();
  StopStreaming();
}

void NetworkRemote::StartStreaming() {
  if (!allow_streaming_ || renderer_registry_) return;

  EngineRouter* router = qobject_cast<EngineRouter*>(app_->player()->engine());
  if (!router) {
    qLog(Error) << "Streaming needs the Player's EngineRouter";
    return;
  }

  qLog(Info) << "Playing on remote devices is allowed";

  // The registry drives engines, so it has to live with the Player.
  renderer_registry_ = new RendererRegistry(app_, router);
  renderer_registry_->moveToThread(router->thread());
  if (CastDiscovery* discovery = app_->cast_discovery()) {
    // Experimental, see --chromecast. The registry is on the Player's thread
    // now, with the discovery.
    RendererRegistry* registry = renderer_registry_;
    QMetaObject::invokeMethod(
        registry,
        [registry, discovery]() { registry->UseCastDevices(discovery); },
        Qt::QueuedConnection);
  }
  media_http_server_.reset(new MediaHttpServer(renderer_registry_->items()));

  connect(incoming_data_parser_.get(),
          SIGNAL(RendererConnected(int, QByteArray, QString, quint16, QString)),
          renderer_registry_,
          SLOT(RegisterRenderer(int, QByteArray, QString, quint16, QString)));
  connect(incoming_data_parser_.get(), SIGNAL(RendererMessage(int, QByteArray)),
          renderer_registry_, SLOT(HandleMessage(int, QByteArray)));
  connect(this, SIGNAL(ClientDisconnected(int)), renderer_registry_,
          SLOT(ClientDisconnected(int)));
  connect(renderer_registry_, SIGNAL(SendToClient(int, QByteArray)), this,
          SLOT(SendToClient(int, QByteArray)));
  connect(renderer_registry_, SIGNAL(SendToAll(QByteArray)), this,
          SLOT(SendToAllClients(QByteArray)));

  incoming_data_parser_->SetStreamingEnabled(true);
  outgoing_data_creator_->SetStreamingEnabled(true);
}

void NetworkRemote::StopStreaming() {
  if (incoming_data_parser_) incoming_data_parser_->SetStreamingEnabled(false);
  if (outgoing_data_creator_) {
    outgoing_data_creator_->SetStreamingEnabled(false);
  }
  media_http_server_.reset();
  if (renderer_registry_) {
    disconnect(renderer_registry_, nullptr, this, nullptr);
    renderer_registry_->deleteLater();
    renderer_registry_ = nullptr;
  }
}

void NetworkRemote::SendToClient(int client_id, const QByteArray& data) {
  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;
  for (RemoteClient* client : clients_) {
    if (client->id() == client_id) {
      client->SendData(&msg);
      return;
    }
  }
}

void NetworkRemote::SendToAllClients(const QByteArray& data) {
  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;
  if (outgoing_data_creator_) outgoing_data_creator_->SendDataToClients(&msg);
}

void NetworkRemote::ReloadSettings() {
  StopServer();
  StartServer();
}

void NetworkRemote::AcceptConnection() {
  if (!signals_connected_) {
    signals_connected_ = true;

    // Setting up the signals, but only once
    connect(incoming_data_parser_.get(), SIGNAL(SendClementineInfo()),
            outgoing_data_creator_.get(), SLOT(SendClementineInfo()));
    connect(incoming_data_parser_.get(), SIGNAL(SendFirstData(bool)),
            outgoing_data_creator_.get(), SLOT(SendFirstData(bool)));
    connect(incoming_data_parser_.get(), SIGNAL(SendAllPlaylists()),
            outgoing_data_creator_.get(), SLOT(SendAllPlaylists()));
    connect(incoming_data_parser_.get(), SIGNAL(SendAllActivePlaylists()),
            outgoing_data_creator_.get(), SLOT(SendAllActivePlaylists()));
    connect(incoming_data_parser_.get(), SIGNAL(SendPlaylistSongs(int)),
            outgoing_data_creator_.get(), SLOT(SendPlaylistSongs(int)));

    connect(app_->playlist_manager(), SIGNAL(ActiveChanged(Playlist*)),
            outgoing_data_creator_.get(), SLOT(ActiveChanged(Playlist*)));
    connect(app_->playlist_manager(), SIGNAL(PlaylistChanged(Playlist*)),
            outgoing_data_creator_.get(), SLOT(PlaylistChanged(Playlist*)));
    connect(app_->playlist_manager(), SIGNAL(PlaylistAdded(int, QString, bool)),
            outgoing_data_creator_.get(),
            SLOT(PlaylistAdded(int, QString, bool)));
    connect(app_->playlist_manager(), SIGNAL(PlaylistRenamed(int, QString)),
            outgoing_data_creator_.get(), SLOT(PlaylistRenamed(int, QString)));
    connect(app_->playlist_manager(), SIGNAL(PlaylistClosed(int)),
            outgoing_data_creator_.get(), SLOT(PlaylistClosed(int)));
    connect(app_->playlist_manager(), SIGNAL(PlaylistDeleted(int)),
            outgoing_data_creator_.get(), SLOT(PlaylistDeleted(int)));

    connect(app_->player(), SIGNAL(VolumeChanged(int)),
            outgoing_data_creator_.get(), SLOT(VolumeChanged(int)));
    connect(app_->player()->engine(), SIGNAL(StateChanged(Engine::State)),
            outgoing_data_creator_.get(), SLOT(StateChanged(Engine::State)));

    connect(app_->playlist_manager()->sequence(),
            SIGNAL(RepeatModeChanged(PlaylistSequence::RepeatMode)),
            outgoing_data_creator_.get(),
            SLOT(SendRepeatMode(PlaylistSequence::RepeatMode)));
    connect(app_->playlist_manager()->sequence(),
            SIGNAL(ShuffleModeChanged(PlaylistSequence::ShuffleMode)),
            outgoing_data_creator_.get(),
            SLOT(SendShuffleMode(PlaylistSequence::ShuffleMode)));

    connect(incoming_data_parser_.get(), SIGNAL(GetLyrics()),
            outgoing_data_creator_.get(), SLOT(GetLyrics()));

    connect(incoming_data_parser_.get(), SIGNAL(SendLibrary(RemoteClient*)),
            outgoing_data_creator_.get(), SLOT(SendLibrary(RemoteClient*)));

    connect(incoming_data_parser_.get(),
            SIGNAL(DoGlobalSearch(QString, RemoteClient*)),
            outgoing_data_creator_.get(),
            SLOT(DoGlobalSearch(QString, RemoteClient*)));

    connect(incoming_data_parser_.get(),
            SIGNAL(SendListFiles(QString, RemoteClient*)),
            outgoing_data_creator_.get(),
            SLOT(SendListFiles(QString, RemoteClient*)));
    connect(incoming_data_parser_.get(), SIGNAL(SendSavedRadios(RemoteClient*)),
            outgoing_data_creator_.get(), SLOT(SendSavedRadios(RemoteClient*)));
  }

  QTcpServer* server = qobject_cast<QTcpServer*>(sender());
  QTcpSocket* client_socket = server->nextPendingConnection();
  // Check if our ip is in private scope
  if (only_non_public_ip_ &&
      !IsLocalClient(client_socket->peerAddress(),
                     client_socket->localAddress(),
                     listen_on_all_addresses_ ? Listening::OnAllAddresses
                                              : Listening::OnChosenAddress)) {
    qLog(Warning) << "Refusing a connection from"
                  << client_socket->peerAddress().toString() << "to"
                  << client_socket->localAddress().toString()
                  << "because only connections from the local network are "
                     "allowed";
    client_socket->setProperty(kRefusedProperty, true);
  }

  // Drop connections that never say anything. Once the socket has been
  // handed on, its new owner is responsible for it.
  QTimer::singleShot(kFirstByteTimeoutMsec, client_socket, [client_socket]() {
    if (client_socket->property(kSniffedProperty).toBool()) return;
    qLog(Debug) << "Dropping a silent connection from"
                << client_socket->peerAddress().toString();
    client_socket->abort();
    client_socket->deleteLater();
  });
  connect(client_socket, &QTcpSocket::readyRead, this,
          [this, client_socket]() { SniffProtocol(client_socket); });
  if (client_socket->bytesAvailable() > 0) SniffProtocol(client_socket);
}

void NetworkRemote::SniffProtocol(QTcpSocket* socket) {
  char first_byte = 0;
  if (socket->property(kSniffedProperty).toBool() ||
      socket->peek(&first_byte, 1) != 1) {
    return;
  }
  socket->setProperty(kSniffedProperty, true);
  disconnect(socket, &QTcpSocket::readyRead, this, nullptr);

  const ProtocolSniffer::Protocol protocol =
      ProtocolSniffer::Classify(static_cast<unsigned char>(first_byte));
  if (socket->property(kRefusedProperty).toBool()) {
    RefuseNotLocal(socket, protocol);
    return;
  }

  switch (protocol) {
    case ProtocolSniffer::Remote: {
      CreateRemoteClient(socket);
      // The first message is already waiting, and readyRead won't be emitted
      // again for it.
      QMetaObject::invokeMethod(clients_.last(), "IncomingData",
                                Qt::QueuedConnection);
      return;
    }

    case ProtocolSniffer::Http:
      if (media_http_server_) {
        media_http_server_->HandleConnection(socket);
        return;
      }
      break;

    case ProtocolSniffer::Unknown:
      break;
  }

  qLog(Debug) << "Closing a connection with an unexpected protocol from"
              << socket->peerAddress().toString();
  socket->abort();
  socket->deleteLater();
}

void NetworkRemote::RefuseNotLocal(QTcpSocket* socket,
                                   ProtocolSniffer::Protocol protocol) {
  // Closing with unread data makes the kernel reset the connection, which can
  // throw away the reply before the client reads it.
  socket->readAll();

  connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
  // In case the client never takes the reply.
  QTimer::singleShot(kFirstByteTimeoutMsec, socket, [socket]() {
    socket->abort();
    socket->deleteLater();
  });

  switch (protocol) {
    case ProtocolSniffer::Remote:
      socket->write(DisconnectMessage(cpb::remote::Not_Local_Network));
      socket->disconnectFromHost();
      return;

    case ProtocolSniffer::Http:
      MediaHttpServer::WriteError(socket, 403, "Forbidden");
      return;

    case ProtocolSniffer::Unknown:
      socket->abort();
      return;
  }
}

QByteArray NetworkRemote::DisconnectMessage(
    cpb::remote::ReasonDisconnect reason) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::DISCONNECT);
  msg.set_version(msg.default_instance().version());
  msg.mutable_response_disconnect()->set_reason_disconnect(reason);
  const std::string data = msg.SerializeAsString();

  // As RemoteClient frames it: a big-endian length, then the message.
  QByteArray framed;
  QDataStream s(&framed, QIODevice::WriteOnly);
  s << qint32(data.length());
  s.writeRawData(data.data(), data.length());
  return framed;
}

bool NetworkRemote::IpIsPrivate(const QHostAddress& address) {
  return
      // Localhost, including v4 mapped to v6 (::ffff:127.0.0.1), which is how
      // v4 clients arrive on a dual-stack socket
      address.isLoopback() ||
      // Localhost v4
      address.isInSubnet(QHostAddress::parseSubnet("127.0.0.0/8")) ||
      // Link Local v4
      address.isInSubnet(QHostAddress::parseSubnet("169.254.0.0/16")) ||
      address.isInSubnet(QHostAddress::parseSubnet("::ffff:169.254.0.0/112")) ||
      // Localhost and Link Local v6
      address.isInSubnet(QHostAddress::parseSubnet("::1/128")) ||
      address.isInSubnet(QHostAddress::parseSubnet("fe80::/10")) ||
      // Private v4 range
      address.isInSubnet(QHostAddress::parseSubnet("192.168.0.0/16")) ||
      address.isInSubnet(QHostAddress::parseSubnet("172.16.0.0/12")) ||
      address.isInSubnet(QHostAddress::parseSubnet("10.0.0.0/8")) ||
      // Private v4 range translated to v6
      address.isInSubnet(QHostAddress::parseSubnet("::ffff:192.168.0.0/112")) ||
      address.isInSubnet(QHostAddress::parseSubnet("::ffff:172.16.0.0/108")) ||
      address.isInSubnet(QHostAddress::parseSubnet("::ffff:10.0.0.0/104")) ||
      // Private v6 range
      address.isInSubnet(QHostAddress::parseSubnet("fc00::/7"));
}

bool NetworkRemote::IsLocalClient(const QHostAddress& peer,
                                  const QHostAddress& local,
                                  Listening listening) {
  if (IpIsPrivate(peer)) return true;
  if (listening != Listening::OnChosenAddress) return false;

  // Arriving on the address isn't enough by itself: a machine in a router's
  // DMZ gets the internet on its LAN address.
  for (const char* text : kSharedSubnets) {
    const QPair<QHostAddress, int> subnet = QHostAddress::parseSubnet(text);
    if (local.isInSubnet(subnet) && peer.isInSubnet(subnet)) return true;
  }
  return false;
}

bool NetworkRemote::LocalClientsCanReach(const QHostAddress& address) {
  return IpIsPrivate(address) || InSharedSubnet(address);
}

void NetworkRemote::CreateRemoteClient(QTcpSocket* client_socket) {
  if (client_socket) {
    // Add the client to the list
    RemoteClient* client =
        new RemoteClient(app_, client_socket, auth_limiter_.get());
    clients_.push_back(client);

    // Update the Remote Root Files for the latest Client
    outgoing_data_creator_->SetMusicExtensions(
        client->files_music_extensions());
    outgoing_data_creator_->SetRemoteRootFiles(client->files_root_folder());
    incoming_data_parser_->SetRemoteRootFiles(client->files_root_folder());
    // update OutgoingDataCreator with latest allow_downloads setting
    outgoing_data_creator_->SetAllowDownloads(client->allow_downloads());

    // Connect the signal to parse data
    connect(client, SIGNAL(Parse(cpb::remote::Message)),
            incoming_data_parser_.get(), SLOT(Parse(cpb::remote::Message)));
    connect(client, SIGNAL(Disconnected(int)), SIGNAL(ClientDisconnected(int)));
  }
}

void NetworkRemote::EnableKittens(bool aww) {
  if (outgoing_data_creator_.get()) outgoing_data_creator_->EnableKittens(aww);
}

void NetworkRemote::SendKitten(quint64 id, const QImage& kitten) {
  if (outgoing_data_creator_.get()) outgoing_data_creator_->SendKitten(kitten);
}
