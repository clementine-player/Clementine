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

#include <QBuffer>
#include <QCoreApplication>
#include <QDataStream>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QNetworkProxy>
#include <QSettings>
#include <QTcpServer>
#include <QThread>
#include <QTimer>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "covers/currentartloader.h"
#include "engines/enginerouter.h"
#include "globalsearch/globalsearch.h"
#include "internet/core/internetmodel.h"
#include "internet/internetradio/savedradio.h"
#include "library/librarymodel.h"
#include "networkremote/authattemptlimiter.h"
#include "networkremote/incomingdataparser.h"
#include "networkremote/internetbrowser.h"
#include "networkremote/outgoingdatacreator.h"
#include "networkremote/protocolsniffer.h"
#include "networkremote/remotesmartplaylists.h"
#include "networkremote/streaming/mediahttpserver.h"
#include "networkremote/streaming/rendererregistry.h"
#include "networkremote/sync/mediasync.h"
#include "networkremote/sync/syncsourceid.h"
#include "networkremote/zeroconf.h"
#include "playlist/playlist.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlistmanager.h"

const char* NetworkRemote::kSettingsGroup = "NetworkRemote";
const quint16 NetworkRemote::kDefaultServerPort = 5500;
const char* NetworkRemote::kTranscoderSettingPostfix = "/NetworkRemote";

namespace {
// How long a new connection may stay silent before it's dropped.
const int kFirstByteTimeoutMsec = 10000;
// How often to check whether the chosen networks' addresses have changed.
const int kAddressCheckMsec = 30000;
const int kMaxPendingSockets = 32;
const int kMaxRemoteClients = 32;
const int kPlaylistUpdateCoalesceMsec = 50;
const int kMaxPendingGlobalSearches = 256;
const char* kSniffedProperty = "clementine_sniffed";
// Set on a connection that isn't from the local network when only those are
// allowed. It's told why once it's said which protocol it speaks.
const char* kRefusedProperty = "clementine_refused";

// Non-public ranges that aren't a local network by themselves. Tailscale
// gives its devices addresses from carrier-grade NAT space, so a listen
// address chosen there makes the rest of it local. Only a chosen one: when
// listening on everything, 100.x clients stay public, as before.
const char* kSharedSubnets[] = {"100.64.0.0/10", "::ffff:100.64.0.0/106"};

// The backend part of the library's smart playlists' sync source ids:
// "smart/library/<uuid>".
const char* kLibrarySmartPlaylists = "library";

// From --remote-name; empty for the host name.
QString sRemoteName;

// From --media-sync.
bool sMediaSyncEnabled = false;

bool InSharedSubnet(const QHostAddress& address) {
  for (const char* subnet : kSharedSubnets) {
    if (address.isInSubnet(QHostAddress::parseSubnet(subnet))) return true;
  }
  return false;
}

QByteArray SerializeRemoteMessage(cpb::remote::Message* msg) {
  msg->set_version(msg->default_instance().version());
  const std::string data = msg->SerializeAsString();
  return QByteArray(data.data(), static_cast<int>(data.size()));
}
}  // namespace

RemoteBridge::RemoteBridge(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      has_clients_(false),
      playlist_update_timer_(new QTimer(this)) {
  qRegisterMetaType<Engine::State>("Engine::State");
  qRegisterMetaType<PlaylistSequence::RepeatMode>(
      "PlaylistSequence::RepeatMode");
  qRegisterMetaType<PlaylistSequence::ShuffleMode>(
      "PlaylistSequence::ShuffleMode");
  qRegisterMetaType<Song>("Song");
  qRegisterMetaType<SongList>("SongList");

  playlist_update_timer_->setSingleShot(true);
  playlist_update_timer_->setInterval(kPlaylistUpdateCoalesceMsec);
  connect(playlist_update_timer_, &QTimer::timeout, this,
          &RemoteBridge::FlushPlaylistUpdates);
}

RemoteBridge::~RemoteBridge() { DisconnectAll(); }

void RemoteBridge::DisconnectAll() {
  for (const QMetaObject::Connection& conn : connections_) {
    disconnect(conn);
  }
  connections_.clear();
}

void RemoteBridge::Shutdown() {
  // Disconnect all signal edges before clearing app_ and scheduling deletion
  // so pending main-thread signals cannot fire into a dying NetworkRemote.
  playlist_update_timer_->stop();
  dirty_playlists_.clear();
  DisconnectAll();
  disconnect();
  app_.clear();
  has_clients_ = false;
  deleteLater();
}

void RemoteBridge::Init() {
  if (!app_) return;
  // RemoteBridge is constructed on the remote thread; parent it to app_ once
  // moved to the main thread so ~Application() can clean up stray bridges.
  if (!parent() && QThread::currentThread() == app_->thread()) {
    setParent(app_);
  }
  DisconnectAll();

  PlaylistManager* playlists = app_->playlist_manager();
  Player* player = app_->player();

  connections_ << connect(playlists, &PlaylistManager::ActiveChanged, this,
                          &RemoteBridge::OnActiveChanged);
  connections_ << connect(playlists, &PlaylistManager::PlaylistChanged, this,
                          &RemoteBridge::OnPlaylistChanged);
  connections_ << connect(playlists, &PlaylistManager::PlaylistAdded, this,
                          &RemoteBridge::OnPlaylistsListChanged);
  connections_ << connect(playlists, &PlaylistManager::PlaylistDeleted, this,
                          &RemoteBridge::OnPlaylistsListChanged);
  connections_ << connect(playlists, &PlaylistManager::PlaylistClosed, this,
                          &RemoteBridge::OnPlaylistsListChanged);
  connections_ << connect(playlists, &PlaylistManager::PlaylistRenamed, this,
                          &RemoteBridge::OnPlaylistsListChanged);

  connections_ << connect(playlists->sequence(),
                          &PlaylistSequence::RepeatModeChanged, this,
                          &RemoteBridge::RepeatModeChanged);
  connections_ << connect(playlists->sequence(),
                          &PlaylistSequence::ShuffleModeChanged, this,
                          &RemoteBridge::ShuffleModeChanged);

  connections_ << connect(player, &Player::VolumeChanged, this,
                          &RemoteBridge::VolumeChanged);
  connections_ << connect(player->engine(), &Engine::Base::StateChanged, this,
                          &RemoteBridge::EngineStateChanged);
  connections_ << connect(app_->current_art_loader(),
                          &CurrentArtLoader::ArtLoaded, this,
                          &RemoteBridge::OnArtLoaded);

  if (GlobalSearch* gs = app_->global_search()) {
    gs->ReloadSettings();
    connections_ << connect(gs, &GlobalSearch::ResultsAvailable, this,
                            &RemoteBridge::OnGlobalSearchResults);
    connections_ << connect(gs, &GlobalSearch::SearchFinished, this,
                            &RemoteBridge::OnGlobalSearchFinished);
  }

  emit EngineStateChanged(player->engine()->state());
  if (PlaylistItemPtr item = player->GetCurrentItem()) {
    current_song_ = item->Metadata();
    emit CurrentSongChanged(current_song_, QString(), QImage(),
                            ActiveCurrentRow(),
                            DownloadabilityOf(current_song_));
  }
}

void RemoteBridge::SetHasClients(bool has_clients) {
  has_clients_ = has_clients;
}

int RemoteBridge::ActiveCurrentRow() const {
  if (!app_) return -1;
  Playlist* active = app_->playlist_manager()->active();
  return active ? active->current_row() : -1;
}

int RemoteBridge::DownloadabilityOf(const Song& song) const {
  if (!app_ || !NetworkRemote::MediaSyncEnabled()) {
    return cpb::remote::DOWNLOADABILITY_UNSPECIFIED;
  }
  return OutgoingDataCreator::DownloadabilityOf(
      song, app_->player()->HandlerForUrl(song.url()));
}

void RemoteBridge::OnArtLoaded(const Song& song, const QString& uri,
                               const QImage& image) {
  current_song_ = song;
  current_uri_ = uri;
  current_image_ = image;
  emit CurrentSongChanged(song, uri, image, ActiveCurrentRow(),
                          DownloadabilityOf(song));
}

void RemoteBridge::QueryClementineInfo(int client_id) {
  const Engine::State state = app_ ? app_->player()->GetState() : Engine::Empty;
  emit ClementineInfoStateReady(client_id, state);
}

void RemoteBridge::QueryFirstData(int client_id, bool send_playlist_songs) {
  if (!app_) return;
  Player* player = app_->player();
  PlaylistManager* pm = app_->playlist_manager();
  Song current_song = current_song_;
  if (PlaylistItemPtr item = player->GetCurrentItem()) {
    current_song = item->Metadata();
  }
  const int row = ActiveCurrentRow();
  const int dl = DownloadabilityOf(current_song);
  const Engine::State state = player->GetState();
  const int volume = player->GetVolume();
  const qint64 position_nanosec = player->engine()->position_nanosec();
  const QByteArray playlists_pb = BuildPlaylistsProto(false);
  const QByteArray playlist_songs_pb =
      send_playlist_songs ? BuildPlaylistSongsProto(pm->active_id())
                          : QByteArray();
  const PlaylistSequence::RepeatMode repeat = pm->sequence()->repeat_mode();
  const PlaylistSequence::ShuffleMode shuffle = pm->sequence()->shuffle_mode();

  emit FirstDataReady(client_id, current_song, row, dl, state, volume,
                      position_nanosec, playlists_pb, playlist_songs_pb, repeat,
                      shuffle);
}

QByteArray RemoteBridge::BuildPlaylistsProto(bool include_closed) const {
  if (!app_) return QByteArray();
  PlaylistManager* playlist_manager = app_->playlist_manager();
  const int active_playlist = playlist_manager->active_id();

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::PLAYLISTS);
  cpb::remote::ResponsePlaylists* playlists = msg.mutable_response_playlists();

  if (include_closed) {
    playlists->set_include_closed(true);
    for (const PlaylistBackend::Playlist& p :
         app_->playlist_backend()->GetAllPlaylists()) {
      const bool playlist_open = playlist_manager->IsPlaylistOpen(p.id);
      const int item_count =
          playlist_open ? playlist_manager->playlist(p.id)->rowCount() : 0;

      cpb::remote::Playlist* pb_pl = playlists->add_playlist();
      pb_pl->set_name(DataCommaSizeFromQString(p.name));
      pb_pl->set_id(p.id);
      pb_pl->set_active(p.id == active_playlist);
      pb_pl->set_item_count(item_count);
      pb_pl->set_closed(!playlist_open);
      pb_pl->set_favorite(p.favorite);
    }
  } else {
    for (Playlist* p : playlist_manager->GetAllPlaylists()) {
      const QString playlist_name = playlist_manager->GetPlaylistName(p->id());
      cpb::remote::Playlist* pb_pl = playlists->add_playlist();
      pb_pl->set_name(DataCommaSizeFromQString(playlist_name));
      pb_pl->set_id(p->id());
      pb_pl->set_active(p->id() == active_playlist);
      pb_pl->set_item_count(p->rowCount());
      pb_pl->set_closed(false);
      pb_pl->set_favorite(p->is_favorite());
    }
  }

  return SerializeRemoteMessage(&msg);
}

QByteArray RemoteBridge::BuildPlaylistSongsProto(int id) const {
  if (!app_) return QByteArray();
  Playlist* playlist = app_->playlist_manager()->playlist(id);
  if (!playlist) {
    qLog(Info) << "Could not find playlist with id =" << id;
    return QByteArray();
  }

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::PLAYLIST_SONGS);
  cpb::remote::ResponsePlaylistSongs* pb_response =
      msg.mutable_response_playlist_songs();
  pb_response->mutable_requested_playlist()->set_id(id);

  int index = 0;
  QImage null_img;
  for (const Song& song : playlist->GetAllSongs()) {
    cpb::remote::SongMetadata* pb_song = pb_response->add_songs();
    OutgoingDataCreator::CreateSong(song, null_img, index, pb_song);
    if (NetworkRemote::MediaSyncEnabled() && pb_song->has_id()) {
      pb_song->set_downloadable(
          static_cast<cpb::remote::Downloadability>(DownloadabilityOf(song)));
    }
    ++index;
  }
  return SerializeRemoteMessage(&msg);
}

void RemoteBridge::QueryPlaylists(int client_id, bool include_closed) {
  const QByteArray data = BuildPlaylistsProto(include_closed);
  if (!data.isEmpty()) emit SendRawToClient(client_id, data);
}

void RemoteBridge::QueryPlaylistSongs(int client_id, int playlist_id) {
  const QByteArray data = BuildPlaylistSongsProto(playlist_id);
  if (!data.isEmpty()) emit SendRawToClient(client_id, data);
}

void RemoteBridge::OnActiveChanged(Playlist* playlist) {
  if (!has_clients_ || !playlist) return;
  // Drop any pending coalesced update for this playlist; its songs are sent
  // immediately below.
  dirty_playlists_.remove(playlist->id());
  const QByteArray songs = BuildPlaylistSongsProto(playlist->id());
  if (!songs.isEmpty()) emit SendRawToAllClients(songs);

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::ACTIVE_PLAYLIST_CHANGED);
  msg.mutable_response_active_changed()->set_id(playlist->id());
  emit SendRawToAllClients(SerializeRemoteMessage(&msg));
}

void RemoteBridge::OnPlaylistChanged(Playlist* playlist) {
  if (!has_clients_ || !playlist) return;
  // Coalesce burst edits into one PLAYLIST_SONGS snapshot per playlist.
  dirty_playlists_.insert(playlist->id());
  if (!playlist_update_timer_->isActive()) {
    playlist_update_timer_->start();
  }
}

void RemoteBridge::FlushPlaylistUpdates() {
  if (!has_clients_ || dirty_playlists_.isEmpty()) {
    dirty_playlists_.clear();
    return;
  }
  const QSet<int> ids = dirty_playlists_;
  dirty_playlists_.clear();
  for (int id : ids) {
    const QByteArray songs = BuildPlaylistSongsProto(id);
    if (!songs.isEmpty()) emit SendRawToAllClients(songs);
  }
}

void RemoteBridge::OnPlaylistsListChanged() {
  if (!has_clients_) return;
  const QByteArray data = BuildPlaylistsProto(false);
  if (!data.isEmpty()) emit SendRawToAllClients(data);
}

void RemoteBridge::QueryTrackPosition() {
  if (!app_) return;
  emit TrackPositionReady(app_->player()->engine()->position_nanosec());
}

void RemoteBridge::StartGlobalSearch(int client_id, const QString& query) {
  if (!app_) return;
  const int id = app_->global_search()->SearchAsync(query);
  // Bound the table if search providers never emit SearchFinished.
  if (global_searches_.size() >= kMaxPendingGlobalSearches) {
    global_searches_.clear();
  }
  global_searches_.insert(id, GlobalSearchEntry{client_id, query});

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::GLOBAL_SEARCH_STATUS);
  cpb::remote::ResponseGlobalSearchStatus* status =
      msg.mutable_response_global_search_status();
  status->set_id(id);
  status->set_query(DataCommaSizeFromQString(query));
  status->set_status(cpb::remote::GlobalSearchStarted);
  emit SendRawToClient(client_id, SerializeRemoteMessage(&msg));
}

void RemoteBridge::OnGlobalSearchResults(
    int id, const SearchProvider::ResultList& results) {
  if (!global_searches_.contains(id) || results.isEmpty()) return;
  if (!results.first().provider_) return;

  const GlobalSearchEntry search_request = global_searches_.value(id);

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::GLOBAL_SEARCH_RESULT);
  cpb::remote::ResponseGlobalSearch* response =
      msg.mutable_response_global_search();
  response->set_id(id);
  response->set_query(DataCommaSizeFromQString(search_request.query));
  response->set_search_provider(
      DataCommaSizeFromQString(results.first().provider_->name()));

  const QImage icon_image(results.first().provider_->icon_as_image());
  QByteArray byte_array;
  QBuffer buf(&byte_array);
  buf.open(QIODevice::WriteOnly);
  icon_image.save(&buf, "PNG");
  response->set_search_provider_icon(byte_array.constData(), byte_array.size());

  QImage null_img;
  for (const SearchProvider::Result& result : results) {
    cpb::remote::SongMetadata* pb_song = response->add_song_metadata();
    OutgoingDataCreator::CreateSong(result.metadata_, null_img, 0, pb_song);
    if (NetworkRemote::MediaSyncEnabled() && pb_song->has_id()) {
      pb_song->set_downloadable(static_cast<cpb::remote::Downloadability>(
          DownloadabilityOf(result.metadata_)));
    }
  }
  emit SendRawToClient(search_request.client_id, SerializeRemoteMessage(&msg));
}

void RemoteBridge::OnGlobalSearchFinished(int id) {
  if (!global_searches_.contains(id)) return;
  const GlobalSearchEntry req = global_searches_.take(id);

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::GLOBAL_SEARCH_STATUS);
  cpb::remote::ResponseGlobalSearchStatus* status =
      msg.mutable_response_global_search_status();
  status->set_id(id);
  status->set_query(DataCommaSizeFromQString(req.query));
  status->set_status(cpb::remote::GlobalSearchFinished);
  emit SendRawToClient(req.client_id, SerializeRemoteMessage(&msg));
}

void RemoteBridge::QuerySavedRadios(int client_id) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::REQUEST_SAVED_RADIOS);

  SavedRadio* radio_service = static_cast<SavedRadio*>(
      InternetModel::ServiceByName(SavedRadio::kServiceName));
  if (radio_service) {
    cpb::remote::ResponseSavedRadios* radios =
        msg.mutable_response_saved_radios();
    for (const auto& stream : radio_service->Streams()) {
      cpb::remote::Stream* pb_stream = radios->add_streams();
      pb_stream->set_name(stream.name_.toStdString());
      pb_stream->set_url(stream.url_.toString().toStdString());
      if (!stream.url_logo_.isEmpty())
        pb_stream->set_url_logo(stream.url_logo_.toString().toStdString());
    }
  }
  emit SendRawToClient(client_id, SerializeRemoteMessage(&msg));
}

void RemoteBridge::HandleChangeSong(int playlist_id, int song_index,
                                    bool enqueue_mode) {
  if (!app_) return;
  PlaylistManager* pm = app_->playlist_manager();
  if (!pm->IsPlaylistOpen(playlist_id)) return;
  Playlist* pl = pm->playlist(playlist_id);
  if (!pl || song_index < 0 || song_index >= pl->rowCount()) return;

  if (pm->active_id() != playlist_id) {
    pm->SetActivePlaylist(playlist_id);
  }

  if (!enqueue_mode) {
    app_->player()->PlayAt(song_index, Engine::Manual, false);
  } else {
    pm->Enque(playlist_id, song_index);
    if (app_->player()->GetState() != Engine::Playing) {
      app_->player()->PlayAt(song_index, Engine::Manual, false);
    }
  }
}

void RemoteBridge::SetCurrentPlaylistIfNotPlaying(int playlist_id) {
  if (!app_) return;
  if (app_->player()->GetState() != Engine::Playing) {
    app_->playlist_manager()->SetCurrentPlaylist(playlist_id);
  }
}

void RemoteBridge::PrepareDownloadSongs(int client_id,
                                        const QByteArray& serialized_request) {
  if (!app_) return;
  cpb::remote::RequestDownloadSongs request;
  if (!request.ParseFromArray(serialized_request.constData(),
                              serialized_request.size())) {
    return;
  }

  Song current_song;
  if (PlaylistItemPtr item = app_->player()->GetCurrentItem()) {
    current_song = item->Metadata();
  }
  const int current_row = ActiveCurrentRow();
  SongList playlist_songs;
  if (request.download_item() == cpb::remote::APlaylist) {
    if (Playlist* playlist =
            app_->playlist_manager()->playlist(request.playlist_id())) {
      playlist_songs = playlist->GetAllSongs();
    }
  }
  emit PreparedDownloadSongs(client_id, serialized_request, current_song,
                             current_row, playlist_songs);
}

NetworkRemote::NetworkRemote(Application* app, QObject* parent)
    : QObject(parent),
      running_(false),
      address_check_(new QTimer(this)),
      renderer_registry_(nullptr),
      internet_browser_(nullptr),
      smart_playlists_(nullptr),
      media_sync_(nullptr),
      allow_streaming_(false),
      app_(app),
      auth_limiter_(new AuthAttemptLimiter) {
  setObjectName("Network remote");

  address_check_->setInterval(kAddressCheckMsec);
  connect(address_check_, &QTimer::timeout, this, &NetworkRemote::Listen);
}

NetworkRemote::~NetworkRemote() {
  StopServer();
  // Main-thread helpers must be torn down on their own event loop.
  if (bridge_) {
    PostTo(bridge_.data(), &RemoteBridge::Shutdown);
    bridge_.clear();
  }
  if (internet_browser_) internet_browser_->deleteLater();
  if (smart_playlists_) smart_playlists_->deleteLater();
  if (media_sync_) media_sync_->deleteLater();
}

void NetworkRemote::ReadSettings() {
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);
  use_remote_ = s.value("use_remote", false).toBool();
  port_ = s.value("port", kDefaultServerPort).toInt();

  // Use only non public ips must be true be default
  only_non_public_ip_ = s.value("only_non_public_ip", true).toBool();

  listen_on_all_addresses_ = s.value("listen_on_all_addresses", true).toBool();
  if (s.contains("listen_networks")) {
    listen_networks_ = s.value("listen_networks").toStringList();
  } else {
    // Saved by a build that chose addresses. Turn them into their networks
    // once, while they're likely still this computer's, so a new lease
    // doesn't lose them. The addresses stay for that build to read.
    listen_networks_ = NetworksOfAddresses(
        s.value("listen_addresses").toStringList(), PresentAddresses());
    s.setValue("listen_networks", listen_networks_);
  }
  allow_streaming_ = s.value("allow_streaming", false).toBool();

  s.endGroup();
}

void NetworkRemote::SetRemoteName(const QString& name) {
  sRemoteName = name.trimmed();
}

QString NetworkRemote::RemoteName() {
  return sRemoteName.isEmpty() ? QHostInfo::localHostName() : sRemoteName;
}

void NetworkRemote::SetMediaSyncEnabled(bool enabled) {
  sMediaSyncEnabled = enabled;
}

bool NetworkRemote::MediaSyncEnabled() { return sMediaSyncEnabled; }

QList<QHostAddress> NetworkRemote::ListenAddresses(
    bool all, const QStringList& networks, const QList<QHostAddress>& present) {
  if (all) {
    // Any is dual-stack, so it covers IPv6 too. The separate AnyIPv6 server
    // is left from Qt 4, where Any meant IPv4 only; on a dual-stack system
    // it fails to bind alongside Any, which is expected.
    return {QHostAddress(QHostAddress::Any),
            QHostAddress(QHostAddress::AnyIPv6)};
  }

  QList<QPair<QHostAddress, int>> subnets;
  for (const QString& text : networks) {
    const QPair<QHostAddress, int> subnet = QHostAddress::parseSubnet(text);
    if (subnet.first.isNull()) {
      qLog(Warning) << "Ignoring a listen network that doesn't parse:" << text;
      continue;
    }
    subnets << subnet;
  }

  QList<QHostAddress> addresses;
  for (const QHostAddress& address : present) {
    for (const QPair<QHostAddress, int>& subnet : subnets) {
      if (address.isInSubnet(subnet)) {
        addresses << address;
        break;
      }
    }
  }
  return addresses;
}

QList<QNetworkAddressEntry> NetworkRemote::PresentAddresses() {
  QList<QNetworkAddressEntry> entries;
  for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
    if (!(iface.flags() & QNetworkInterface::IsUp)) continue;

    for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
      const QHostAddress address = entry.ip();
      if (address.protocol() == QAbstractSocket::IPv6Protocol &&
          address.isLinkLocal()) {
        continue;
      }
      entries << entry;
    }
  }
  return entries;
}

QString NetworkRemote::NetworkOf(const QHostAddress& address,
                                 int prefix_length) {
  if (address.protocol() == QAbstractSocket::IPv4Protocol) {
    prefix_length = qBound(0, prefix_length, 32);
    const quint32 mask =
        prefix_length == 0 ? 0 : ~quint32(0) << (32 - prefix_length);
    return QString("%1/%2")
        .arg(QHostAddress(address.toIPv4Address() & mask).toString())
        .arg(prefix_length);
  }

  prefix_length = qBound(0, prefix_length, 128);
  Q_IPV6ADDR bytes = address.toIPv6Address();
  for (int i = 0; i < 16; ++i) {
    const int bits = qBound(0, prefix_length - i * 8, 8);
    bytes[i] &= quint8(0xff00 >> bits);
  }
  return QString("%1/%2")
      .arg(QHostAddress(bytes).toString())
      .arg(prefix_length);
}

QString NetworkRemote::ParseNetwork(const QString& text) {
  const QString trimmed = text.trimmed();
  if (!trimmed.contains('/')) {
    const QHostAddress address(trimmed);
    if (address.isNull()) return QString();
    return NetworkOf(
        address,
        address.protocol() == QAbstractSocket::IPv4Protocol ? 32 : 128);
  }

  const QPair<QHostAddress, int> subnet = QHostAddress::parseSubnet(trimmed);
  if (subnet.first.isNull()) return QString();
  return NetworkOf(subnet.first, subnet.second);
}

QStringList NetworkRemote::NetworksOfAddresses(
    const QStringList& addresses, const QList<QNetworkAddressEntry>& present) {
  QStringList networks;
  for (const QString& text : addresses) {
    const QHostAddress address(text);
    if (address.isNull()) continue;

    QString network = ParseNetwork(text);
    for (const QNetworkAddressEntry& entry : present) {
      if (entry.ip() == address && entry.prefixLength() >= 0) {
        network = NetworkOf(address, entry.prefixLength());
        break;
      }
    }
    if (!networks.contains(network)) networks << network;
  }
  return networks;
}

void NetworkRemote::SetupServer() {
  // Retire any prior main-thread helpers on their thread before replacing.
  if (bridge_) {
    PostTo(bridge_.data(), &RemoteBridge::Shutdown);
    bridge_.clear();
  }
  if (internet_browser_) {
    internet_browser_->deleteLater();
    internet_browser_ = nullptr;
  }
  if (smart_playlists_) {
    smart_playlists_->deleteLater();
    smart_playlists_ = nullptr;
  }
  if (media_sync_) {
    media_sync_->deleteLater();
    media_sync_ = nullptr;
  }

  incoming_data_parser_.reset(new IncomingDataParser(app_));
  outgoing_data_creator_.reset(new OutgoingDataCreator(app_));

  outgoing_data_creator_->SetClients(&clients_);

  RemoteBridge* bridge = new RemoteBridge(app_);
  bridge->moveToThread(QCoreApplication::instance()->thread());
  bridge_ = bridge;

  connect(incoming_data_parser_.get(), &IncomingDataParser::SendClementineInfo,
          bridge, &RemoteBridge::QueryClementineInfo);
  connect(bridge, &RemoteBridge::ClementineInfoStateReady,
          outgoing_data_creator_.get(),
          &OutgoingDataCreator::SendClementineInfo);

  connect(incoming_data_parser_.get(), &IncomingDataParser::SendFirstData,
          bridge, &RemoteBridge::QueryFirstData);
  connect(bridge, &RemoteBridge::FirstDataReady, outgoing_data_creator_.get(),
          &OutgoingDataCreator::SendFirstData);

  connect(incoming_data_parser_.get(),
          &IncomingDataParser::SendPlaylistsRequested, bridge,
          &RemoteBridge::QueryPlaylists);
  connect(incoming_data_parser_.get(), &IncomingDataParser::SendPlaylistSongs,
          bridge, &RemoteBridge::QueryPlaylistSongs);
  connect(bridge, &RemoteBridge::SendRawToClient, outgoing_data_creator_.get(),
          &OutgoingDataCreator::SendRawToClient);
  connect(bridge, &RemoteBridge::SendRawToAllClients,
          outgoing_data_creator_.get(),
          &OutgoingDataCreator::SendRawToAllClients);

  connect(bridge, &RemoteBridge::EngineStateChanged,
          outgoing_data_creator_.get(), &OutgoingDataCreator::StateChanged);
  connect(bridge, &RemoteBridge::VolumeChanged, outgoing_data_creator_.get(),
          &OutgoingDataCreator::VolumeChanged);
  connect(bridge, &RemoteBridge::RepeatModeChanged,
          outgoing_data_creator_.get(), &OutgoingDataCreator::SendRepeatMode);
  connect(bridge, &RemoteBridge::ShuffleModeChanged,
          outgoing_data_creator_.get(), &OutgoingDataCreator::SendShuffleMode);
  connect(bridge, &RemoteBridge::CurrentSongChanged,
          outgoing_data_creator_.get(),
          &OutgoingDataCreator::CurrentSongChanged);

  connect(outgoing_data_creator_.get(),
          &OutgoingDataCreator::RequestTrackPosition, bridge,
          &RemoteBridge::QueryTrackPosition);
  connect(bridge, &RemoteBridge::TrackPositionReady,
          outgoing_data_creator_.get(),
          &OutgoingDataCreator::SendTrackPosition);

  connect(incoming_data_parser_.get(), &IncomingDataParser::GetLyrics,
          outgoing_data_creator_.get(), &OutgoingDataCreator::GetLyrics);
  connect(incoming_data_parser_.get(), &IncomingDataParser::SendLibrary,
          outgoing_data_creator_.get(), &OutgoingDataCreator::SendLibrary);
  connect(incoming_data_parser_.get(), &IncomingDataParser::SendListFiles,
          outgoing_data_creator_.get(), &OutgoingDataCreator::SendListFiles);

  connect(incoming_data_parser_.get(), &IncomingDataParser::DoGlobalSearch,
          bridge, &RemoteBridge::StartGlobalSearch);
  connect(incoming_data_parser_.get(), &IncomingDataParser::SendSavedRadios,
          bridge, &RemoteBridge::QuerySavedRadios);

  connect(incoming_data_parser_.get(), &IncomingDataParser::ChangeSongRequested,
          bridge, &RemoteBridge::HandleChangeSong);
  connect(incoming_data_parser_.get(),
          &IncomingDataParser::SetCurrentPlaylistIfNotPlaying, bridge,
          &RemoteBridge::SetCurrentPlaylistIfNotPlaying);

  connect(incoming_data_parser_.get(),
          &IncomingDataParser::DownloadSongsRequested, bridge,
          &RemoteBridge::PrepareDownloadSongs);
  connect(bridge, &RemoteBridge::PreparedDownloadSongs, this,
          &NetworkRemote::HandlePreparedDownload);
  PostTo(bridge, &RemoteBridge::Init);

  connect(incoming_data_parser_.get(), SIGNAL(AddToPlaylistSignal(QMimeData*)),
          SIGNAL(AddToPlaylistSignal(QMimeData*)));

  // Browsing the Internet sidebar. The browser works on the sidebar's model,
  // so it lives on the main thread, and gets the model there.
  internet_browser_ = new InternetBrowser;
  internet_browser_->moveToThread(QCoreApplication::instance()->thread());
  Application* app = app_;
  InternetBrowser* browser = internet_browser_;
  PostTo(internet_browser_, [app, browser]() {
    InternetBrowser::Hooks hooks =
        InternetBrowser::HooksFor(app->internet_model());
    hooks.url_handler = [app](const Song& song) {
      return app->player()->HandlerForUrl(song.url());
    };
    browser->SetModel(app->internet_model()->merged_model(), hooks);
  });
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

  SetupSmartPlaylists();
  if (MediaSyncEnabled()) SetupMediaSync();
}

void NetworkRemote::SetupSmartPlaylists() {
  Application* app = app_;
  RemoteSmartPlaylists::Hooks hooks;
  // Ids are the smart playlists' sync source ids, so that a remote that syncs
  // can keep one by its id.
  hooks.list = [app]() {
    QList<cpb::remote::SmartPlaylist> list;
    for (const LibraryModel::SmartPlaylist& smart :
         app->library_model()->SmartPlaylists()) {
      cpb::remote::SmartPlaylist item;
      item.set_id(
          SyncSourceId::ForSmartPlaylist(kLibrarySmartPlaylists, smart.uuid)
              .ToString()
              .toStdString());
      item.set_name(smart.name.toStdString());
      list << item;
    }
    return list;
  };
  hooks.generator = [app](const QString& id) {
    const SyncSourceId source = SyncSourceId::Parse(id);
    if (source.kind() != SyncSourceId::Kind::SmartPlaylist ||
        source.parts().value(0) != kLibrarySmartPlaylists) {
      return smart_playlists::GeneratorPtr();
    }
    return app->library_model()->CreateGenerator(source.parts().value(1));
  };

  smart_playlists_ = new RemoteSmartPlaylists(hooks);
  smart_playlists_->moveToThread(QCoreApplication::instance()->thread());
  connect(incoming_data_parser_.get(),
          SIGNAL(SmartPlaylistMessage(int, QByteArray)), smart_playlists_,
          SLOT(HandleMessage(int, QByteArray)));
  connect(smart_playlists_, SIGNAL(SendToClient(int, QByteArray)), this,
          SLOT(SendToClient(int, QByteArray)));
  connect(smart_playlists_, SIGNAL(AddToPlaylist(QMimeData*)), this,
          SIGNAL(AddToPlaylistSignal(QMimeData*)));
}

void NetworkRemote::SetupMediaSync() {
  Application* app = app_;
  MediaSync::Hooks hooks;
  hooks.playlists.songs = [app](int id, SongList* songs) {
    // An open playlist as it is now, with changes not yet saved.
    if (Playlist* open = app->playlist_manager()->playlist(id)) {
      *songs = open->GetAllSongs();
      return true;
    }
    PlaylistBackend* backend = app->playlist_backend();
    if (backend->GetPlaylist(id).id != id) return false;
    *songs = backend->GetPlaylistSongs(id);
    return true;
  };
  hooks.smart_playlists.generator = [app](const QString& backend,
                                          const QString& uuid) {
    // Only the library's: services that have them are synced through the
    // Internet tab.
    if (backend != kLibrarySmartPlaylists) {
      return smart_playlists::GeneratorPtr();
    }
    return app->library_model()->CreateGenerator(uuid);
  };
  hooks.sources = [app]() {
    QList<cpb::remote::SyncSource> sources;
    PlaylistManager* manager = app->playlist_manager();
    for (const Playlist* playlist : manager->GetAllPlaylists()) {
      cpb::remote::SyncSource source;
      source.set_source_id(
          SyncSourceId::ForPlaylist(playlist->id()).ToString().toStdString());
      source.set_title(manager->GetPlaylistName(playlist->id()).toStdString());
      source.set_kind(cpb::remote::SYNC_SOURCE_KIND_PLAYLIST);
      sources << source;
    }
    for (const LibraryModel::SmartPlaylist& smart :
         app->library_model()->SmartPlaylists()) {
      cpb::remote::SyncSource source;
      source.set_source_id(
          SyncSourceId::ForSmartPlaylist(kLibrarySmartPlaylists, smart.uuid)
              .ToString()
              .toStdString());
      source.set_title(smart.name.toStdString());
      source.set_kind(cpb::remote::SYNC_SOURCE_KIND_SMART_PLAYLIST);
      sources << source;
    }
    return sources;
  };
  hooks.url_handler = [app](const Song& song) {
    return app->player()->HandlerForUrl(song.url());
  };
  hooks.downloads_allowed = []() {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    return s.value("allow_downloads", false).toBool();
  };

  media_sync_ = new MediaSync(app_->library_backend(), hooks,
                              std::make_shared<StreamItemTable>());
  media_sync_->moveToThread(QCoreApplication::instance()->thread());
  connect(incoming_data_parser_.get(),
          SIGNAL(SyncMessage(int, QString, QByteArray)), media_sync_,
          SLOT(HandleMessage(int, QString, QByteArray)));
  connect(this, SIGNAL(ClientDisconnected(int)), media_sync_,
          SLOT(ClientDisconnected(int)));
  connect(media_sync_, SIGNAL(SendToClient(int, QByteArray)), this,
          SLOT(SendToClient(int, QByteArray)));
}

void NetworkRemote::StartServer() {
  if (!app_) {
    qLog(Error) << "Start Server called without having an application!";
    return;
  }
  if (!renderer_registry_) renderer_registry_ = app_->renderer_registry();

  // Check if user desires to start a network remote server
  ReadSettings();
  if (!use_remote_) {
    qLog(Info) << "Network Remote deactivated";
    PublishServing(QList<QHostAddress>());
    return;
  }

  if (running_) {
    // Already running; ReloadSettings stops the servers before restarting.
    return;
  }

  qLog(Info) << "Starting network remote";

  // Cast devices and the like fetch media over HTTP on the same port.
  if (renderer_registry_) {
    media_http_server_.reset(new MediaHttpServer(
        renderer_registry_->items(), renderer_registry_->event_callbacks()));
    if (media_sync_) media_http_server_->SetSyncItems(media_sync_->items());
    media_http_server_->SetSleepInhibitor(app_->sleep_inhibitor());
  }
  // Remotes can sync once there's somewhere to download from.
  outgoing_data_creator_->SetSyncEnabled(media_sync_ && media_http_server_);
  StartStreaming();

  running_ = true;
  Bind();
  if (!listen_on_all_addresses_ && listen_networks_.isEmpty()) {
    qLog(Warning) << "Network remote enabled, but no listen network is chosen";
  } else if (wanted_.isEmpty()) {
    qLog(Info) << "This computer isn't on any of the chosen networks yet";
  }
  Advertise();
  if (!listen_on_all_addresses_) address_check_->start();
}

void NetworkRemote::Listen() {
  if (Bind()) Advertise();
}

bool NetworkRemote::Bind() {
  QList<QHostAddress> present;
  if (!listen_on_all_addresses_) {
    for (const QNetworkAddressEntry& entry : PresentAddresses()) {
      present << entry.ip();
    }
  }
  const QList<QHostAddress> wanted =
      ListenAddresses(listen_on_all_addresses_, listen_networks_, present);

  bool changed = false;
  QList<QHostAddress> bound;
  for (auto it = servers_.begin(); it != servers_.end();) {
    QTcpServer* server = it->get();
    if (!server->isListening()) {
      it = servers_.erase(it);
      continue;
    }
    if (wanted.contains(server->serverAddress())) {
      bound << server->serverAddress();
      ++it;
      continue;
    }
    qLog(Info) << "No longer listening on" << server->serverAddress().toString()
               << "port" << port_;
    server->close();
    it = servers_.erase(it);
    changed = true;
  }

  for (const QHostAddress& address : wanted) {
    if (bound.contains(address)) continue;

    std::unique_ptr<QTcpServer> server(new QTcpServer);
    server->setProxy(QNetworkProxy::NoProxy);
    connect(server.get(), SIGNAL(newConnection()), this,
            SLOT(AcceptConnection()));

    if (server->listen(address, port_)) {
      qLog(Info) << "Listening on" << address.toString() << "port" << port_;
      bound << address;
      servers_.push_back(std::move(server));
      changed = true;
    } else if (listen_on_all_addresses_ || wanted_.contains(address)) {
      // See ListenAddresses: one of the two wildcard servers failing is
      // normal. Otherwise it's failed before, and is tried again on every
      // check.
      qLog(Debug) << "Couldn't listen on" << address.toString() << "port"
                  << port_ << ":" << server->errorString();
    } else {
      qLog(Warning) << "Couldn't listen on" << address.toString() << "port"
                    << port_ << ":" << server->errorString();
    }
  }

  wanted_ = wanted;
  listening_.clear();
  for (const QHostAddress& address : wanted) {
    if (bound.contains(address)) listening_ << address;
  }
  return changed;
}

void NetworkRemote::Advertise() {
  PublishServing(listening_);

  if (Zeroconf::GetZeroconf()) {
    // Advertise only where something is listening, so remotes aren't sent to
    // an address that refuses them. That includes a chosen address that
    // couldn't be bound, such as a VPN's while it's down.
    if (!listen_on_all_addresses_ && listening_.isEmpty()) {
      qLog(Warning) << "Not listening on any address, so not advertising";
      Zeroconf::GetZeroconf()->Unpublish();
    } else {
      QString name = QString("Clementine on %1").arg(RemoteName());
      Zeroconf::GetZeroconf()->Publish(
          "local", "_clementine._tcp", name, port_,
          listen_on_all_addresses_ ? QList<QHostAddress>() : listening_);
    }
  }
}

void NetworkRemote::StopServer() {
  if (!running_) return;
  running_ = false;
  address_check_->stop();

  if (Zeroconf::GetZeroconf()) {
    Zeroconf::GetZeroconf()->Unpublish();
  }

  if (outgoing_data_creator_) {
    outgoing_data_creator_->DisconnectAllClients();
  }
  // Deleting a server closes it.
  servers_.clear();
  listening_.clear();
  wanted_.clear();
  // Snapshot and clear clients_ before notifying downstream listeners so
  // re-entrant callbacks cannot observe deleted clients, and emit
  // ClientDisconnected explicitly before StopStreaming() disconnects slots.
  const QList<RemoteClient*> old_clients = clients_;
  clients_.clear();
  for (RemoteClient* client : old_clients) {
    emit ClientDisconnected(client->id());
    // Give the Server_Shutdown disconnect frame a brief window to flush.
    client->FlushBeforeDelete(200);
    delete client;
  }
  if (media_sync_) PostTo(media_sync_, &MediaSync::ClearClients);
  if (bridge_) PostTo(bridge_.data(), &RemoteBridge::SetHasClients, false);
  StopStreaming();
  media_http_server_.reset();
  PublishServing(QList<QHostAddress>());
}

void NetworkRemote::PublishServing(const QList<QHostAddress>& listening) {
  if (!renderer_registry_) return;

  RendererRegistry::Serving serving;
  serving.enabled = running_;
  serving.port = listening.isEmpty() ? 0 : port_;
  serving.configured_port = running_ ? port_ : 0;
  if (!listen_on_all_addresses_) serving.addresses = listening;
  serving.only_local = only_non_public_ip_;

  // The registry lives on the Player's thread.
  PostTo(renderer_registry_, &RendererRegistry::SetServing, serving);
}

void NetworkRemote::StartStreaming() {
  if (!allow_streaming_ || !renderer_registry_) return;

  qLog(Info) << "Playing on remote devices is allowed";

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
  if (renderer_registry_) {
    // The registry stays, since it offers Cast devices and the like whether
    // or not the remote's on, but remotes no longer hear about it and any
    // previously connected phone renderers are removed.
    PostTo(renderer_registry_, &RendererRegistry::ClearRenderers);
    disconnect(renderer_registry_, nullptr, this, nullptr);
    disconnect(this, SIGNAL(ClientDisconnected(int)), renderer_registry_,
               nullptr);
    if (incoming_data_parser_) {
      disconnect(incoming_data_parser_.get(), nullptr, renderer_registry_,
                 nullptr);
    }
  }
}

void NetworkRemote::SendToClient(int client_id, const QByteArray& data) {
  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;
  for (RemoteClient* client : clients_) {
    if (client->id() == client_id) {
      if (!client->is_closing()) client->SendData(&msg);
      return;
    }
  }
}

void NetworkRemote::SendToAllClients(const QByteArray& data) {
  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;
  if (outgoing_data_creator_) outgoing_data_creator_->SendDataToClients(&msg);
}

void NetworkRemote::OnClientDisconnected(int client_id) {
  for (int i = 0; i < clients_.size(); ++i) {
    if (clients_[i]->id() == client_id) {
      RemoteClient* client = clients_.takeAt(i);
      // Defer deletion because this runs inside SocketDisconnected() handlers.
      client->deleteLater();
      break;
    }
  }
  if (clients_.isEmpty() && bridge_) {
    PostTo(bridge_.data(), &RemoteBridge::SetHasClients, false);
  }
  emit ClientDisconnected(client_id);
}

void NetworkRemote::HandlePreparedDownload(int client_id,
                                           const QByteArray& serialized_request,
                                           const Song& current_song,
                                           int current_row,
                                           const SongList& playlist_songs) {
  cpb::remote::RequestDownloadSongs request;
  if (!request.ParseFromArray(serialized_request.constData(),
                              serialized_request.size())) {
    return;
  }
  for (RemoteClient* client : clients_) {
    if (client->id() == client_id) {
      if (!client->is_closing()) {
        client->song_sender()->SendSongs(request, current_song, current_row,
                                         playlist_songs);
      }
      return;
    }
  }
}

void NetworkRemote::ReloadSettings() {
  StopServer();
  StartServer();
}

void NetworkRemote::AcceptConnection() {
  QTcpServer* server = qobject_cast<QTcpServer*>(sender());
  if (!server) return;

  while (QTcpSocket* client_socket = server->nextPendingConnection()) {
    // Cap pre-sniff sockets to bound FD/memory use from silent peers.
    const int pending =
        findChildren<QTcpSocket*>(QString(), Qt::FindDirectChildrenOnly).size();
    if (pending >= kMaxPendingSockets) {
      qLog(Warning) << "Too many pending connections; dropping connection from"
                    << client_socket->peerAddress().toString();
      client_socket->abort();
      client_socket->deleteLater();
      continue;
    }

    // Reparent immediately from QTcpServer so closed servers can be freed.
    client_socket->setParent(this);

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
      // Evict the oldest unauthenticated client before rejecting new peers so
      // idle unauthenticated sockets cannot starve real remotes.
      if (clients_.size() >= kMaxRemoteClients) {
        for (int i = 0; i < clients_.size(); ++i) {
          if (!clients_[i]->is_authenticated()) {
            RemoteClient* evicted = clients_.takeAt(i);
            // Disconnect signals first so DisconnectClient() cannot re-enter
            // OnClientDisconnected() on an already-removed client.
            evicted->disconnect(this);
            evicted->DisconnectClient(cpb::remote::Not_Authenticated);
            evicted->FlushBeforeDelete(50);
            emit ClientDisconnected(evicted->id());
            evicted->deleteLater();
            break;
          }
        }
      }
      if (clients_.size() >= kMaxRemoteClients) {
        qLog(Warning) << "Max remote client limit reached; closing"
                      << socket->peerAddress().toString();
        break;
      }
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
    if (bridge_) PostTo(bridge_.data(), &RemoteBridge::SetHasClients, true);

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
    connect(client, &RemoteClient::Disconnected, this,
            &NetworkRemote::OnClientDisconnected);
  }
}

void NetworkRemote::EnableKittens(bool aww) {
  if (outgoing_data_creator_.get()) outgoing_data_creator_->EnableKittens(aww);
}

void NetworkRemote::SendKitten(quint64 id, const QImage& kitten) {
  if (outgoing_data_creator_.get()) outgoing_data_creator_->SendKitten(kitten);
}
