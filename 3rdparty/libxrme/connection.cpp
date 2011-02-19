/* This file is part of the XMPP Remote Media Extension.
   Copyright 2010, David Sansome <me@davidsansome.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include "connection.h"
#include "mediaplayerhandler.h"
#include "remotecontrolhandler.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QtDebug>

#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/connectiontcpclient.h>
#include <gloox/disco.h>
#include <gloox/discohandler.h>
#include <gloox/loghandler.h>
#include <gloox/rosterlistener.h>
#include <gloox/rostermanager.h>

namespace xrme {

struct Connection::Private : public gloox::ConnectionListener,
                             public gloox::LogHandler,
                             public gloox::RosterListener,
                             public gloox::DiscoHandler {
  Private(Connection* parent)
    : parent_(parent),
      server_(kDefaultServer),
      jid_resource_(kDefaultJIDResource),
      jid_host_(kDefaultJIDHost),
      verbose_(false),
      media_player_(NULL),
      remote_control_(NULL),
      spontaneous_disconnect_(true) {}

  static const char* kDefaultServer;
  static const char* kDefaultJIDResource;
  static const char* kDefaultJIDHost;

  Connection* parent_;

  // Stuff the user sets before Connect()
  QString username_;
  QString password_;
  QString agent_name_;
  QString server_;
  QString jid_resource_;
  QString jid_host_;
  bool verbose_;

  // Interfaces
  MediaPlayerInterface* media_player_;
  RemoteControlInterface* remote_control_;
  QList<Handler*> handlers_;

  // Stuff that is valid when we're connected.
  QScopedPointer<gloox::Client> client_;
  QScopedPointer<QSocketNotifier> socket_notifier_;

  // After discovering a peer we query it to find its capabilities.  Only after
  // it replies to the query do we put it in peers_ and emit PeerFound().
  QList<Peer> peers_;
  QList<Peer> querying_peers_;

  bool has_peer(const QString& jid_resource) const;

  // We can't destroy the client_ in the onDisconnect() handler, so we have to
  // do it with a QTimer if we get a spontaneous disconnect.
  bool spontaneous_disconnect_;

  // gloox::MessageHandler
  void handleMessage(gloox::Stanza* stanza, gloox::MessageSession* session = 0);

  // gloox::ConnectionListener
  void onConnect();
  void onDisconnect(gloox::ConnectionError e);
  bool onTLSConnect(const gloox::CertInfo& info);

  // gloox::LogHandler
  void handleLog(gloox::LogLevel level, gloox::LogArea area,
                 const std::string& message);

  // gloox::IqHandler
  bool handleIq(gloox::Stanza* stanza);
  bool handleIqID(gloox::Stanza* stanza, int context);

  // gloox::RosterListener
  void handleItemAdded(const gloox::JID&) {}
  void handleItemSubscribed(const gloox::JID&) {}
  void handleItemRemoved(const gloox::JID&) {}
  void handleItemUpdated(const gloox::JID&) {}
  void handleItemUnsubscribed(const gloox::JID&) {}
  void handleRoster(const gloox::Roster&) {}
  void handleRosterPresence(const gloox::RosterItem& item, const std::string& resource, gloox::Presence presence, const std::string& msg);
  void handleSelfPresence(const gloox::RosterItem&, const std::string&, gloox::Presence, const std::string&) {}
  bool handleSubscriptionRequest(const gloox::JID&, const std::string&) { return false; }
  bool handleUnsubscriptionRequest(const gloox::JID&, const std::string&) { return false; }
  void handleNonrosterPresence(gloox::Stanza*) {}
  void handleRosterError(gloox::Stanza*) {}

  // gloox::DiscoHandler
  void handleDiscoInfoResult(gloox::Stanza* stanza, int context);
  void handleDiscoItemsResult(gloox::Stanza*, int) {}
  void handleDiscoError(gloox::Stanza* stanza, int context);
};

const char* Connection::Private::kDefaultServer = "talk.google.com";
const char* Connection::Private::kDefaultJIDResource = "xrmeagent";
const char* Connection::Private::kDefaultJIDHost = "gmail.com";


Connection::Connection(QObject* parent)
    : QObject(parent),
      d(new Private(this)) {
}

Connection::~Connection() {
  qDeleteAll(d->handlers_);
}

void Connection::set_username(const QString& username) { d->username_ = username; }
void Connection::set_password(const QString& password) { d->password_ = password; }
void Connection::set_agent_name(const QString& agent_name) { d->agent_name_ = agent_name; }
void Connection::set_server(const QString& server) { d->server_ = server; }
void Connection::set_jid_resource(const QString& resource) { d->jid_resource_ = resource; }
void Connection::set_jid_host(const QString& host) { d->jid_host_ = host; }
void Connection::set_verbose(bool verbose) { d->verbose_ = verbose; }

QString Connection::username() const { return d->username_; }
QString Connection::password() const { return d->password_; }
QString Connection::agent_name() const { return d->agent_name_; }
QString Connection::server() const { return d->server_; }
QString Connection::jid_resource() const { return d->jid_resource_; }
QString Connection::jid_host() const { return d->jid_host_; }
bool Connection::is_verbose() const { return d->verbose_; }

void Connection::SetMediaPlayer(MediaPlayerInterface* interface) {
  if (d->media_player_) {
    qWarning() << "Connection::SetMediaPlayer: this connection already has a"
                  " MediaPlayerInterface set";
    return;
  }

  if (!interface) {
    qWarning() << "Connection::SetMediaPlayer called with NULL interface";
    return;
  }

  d->media_player_ = interface;
  d->handlers_ << new MediaPlayerHandler(interface);
}

void Connection::SetRemoteControl(RemoteControlInterface* interface) {
  if (d->media_player_) {
    qWarning() << "Connection::RemoteControlInterface: this connection already"
                  " has a RemoteControlInterface set";
    return;
  }

  if (!interface) {
    qWarning() << "Connection::SetRemoteControl called with NULL interface";
    return;
  }

  d->remote_control_ = interface;
  d->handlers_ << new RemoteControlHandler(interface);
}

bool Connection::is_connected() const {
  return (d->client_ && d->client_->state() == gloox::StateConnected);
}

QString Connection::jid() const {
  if (is_connected()) {
    return QString::fromUtf8(d->client_->jid().full().c_str());
  }
  return QString();
}

Connection::PeerList Connection::peers() const {
  if (is_connected()) {
    return d->peers_;
  }
  return PeerList();
}

Connection::PeerList Connection::peers(Peer::Capability cap) const {
  PeerList ret;
  foreach (const Peer& peer, peers()) {
    if (peer.caps_ & cap) {
      ret << peer;
    }
  }
  return ret;
}

bool Connection::Connect() {
  if (d->username_.isEmpty() || d->password_.isEmpty() || d->agent_name_.isEmpty()) {
    qWarning() << __PRETTY_FUNCTION__
               << ": A required field (username/password/agent_name) was empty";
    return false;
  }

  // Construct the JID - append the default host if the user didn't provide one
  QString jid = d->username_;
  if (!jid.contains('@')) {
    jid.append("@" + d->jid_host_);
  }
  jid.append("/" + d->jid_resource_);

  // Create a new connection
  d->client_.reset(new gloox::Client(gloox::JID(jid.toUtf8().constData()),
                                     d->password_.toUtf8().constData()));
  gloox::ConnectionTCPClient* connection = new gloox::ConnectionTCPClient(
        d->client_.data(), d->client_->logInstance(),
        d->server_.toUtf8().constData());
  d->client_->setConnectionImpl(connection);

  // Add listeners
  d->client_->registerConnectionListener(d.data());
  d->client_->rosterManager()->registerRosterListener(d.data());
  d->client_->logInstance().registerLogHandler(
        gloox::LogLevelDebug, gloox::LogAreaAll, d.data());

  // Setup disco
  d->client_->disco()->setIdentity("client", "bot");
  d->client_->disco()->setVersion(d->agent_name_.toUtf8().constData(), std::string());
  d->client_->disco()->addFeature(kXmlnsXrme);

  // Initialise the handlers
  foreach (Handler* handler, d->handlers_) {
    handler->Init(this, d->client_.data());
  }

  // Set presence
  d->client_->setPresence(gloox::PresenceAvailable, -128);

  // Connect
  if (!d->client_->connect(false)) {
    d->client_.reset();
    foreach (Handler* handler, d->handlers_) {
      handler->Reset();
    }
    return false;
  }

  // Listen on the connection's socket
  d->socket_notifier_.reset(new QSocketNotifier(
        connection->socket(), QSocketNotifier::Read));
  connect(d->socket_notifier_.data(), SIGNAL(activated(int)),
          SLOT(SocketReadyReceive()));

  return true;
}

void Connection::Disconnect() {
  if (is_connected()) {
    d->spontaneous_disconnect_ = false;
    d->client_->disconnect();
    d->client_.reset();
    d->spontaneous_disconnect_ = true;
  }
}

void Connection::SocketReadyReceive() {
  d->client_->recv();
}

void Connection::Private::onConnect() {
  parent_->RefreshPeers();
  emit parent_->Connected();
}

void Connection::Private::onDisconnect(gloox::ConnectionError e) {
  QString error_text;
  switch (e) {
    case gloox::ConnNoError:
    case gloox::ConnUserDisconnected:
      break;
    case gloox::ConnStreamError:
      error_text = QString::fromUtf8(client_->streamErrorText().c_str());
      break;
    case gloox::ConnAuthenticationFailed:
      error_text = QString("Authentication error (%1)").arg(client_->authError());
      break;
    default:
      error_text = QString("Unknown error (%1)").arg(e);
      break;
  }

  foreach (Handler* handler, handlers_) {
    handler->Reset();
  }

  socket_notifier_->setEnabled(false);
  socket_notifier_.reset();
  peers_.clear();
  querying_peers_.clear();

  emit parent_->Disconnected(error_text);

  if (spontaneous_disconnect_) {
    QTimer::singleShot(0, parent_, SLOT(CleanupClient()));
  }
}

void Connection::CleanupClient() {
  d->client_.reset();
}

bool Connection::Private::onTLSConnect(const gloox::CertInfo& info) {
  return true;
}

void Connection::Private::handleLog(gloox::LogLevel level, gloox::LogArea area,
                     const std::string& message) {
  if (!verbose_) {
    return;
  }

  QString prefix = "---";
  if (area == gloox::LogAreaXmlIncoming) {
    prefix = "<<<";
  } else if (area == gloox::LogAreaXmlOutgoing) {
    prefix = ">>>";
  }

  qDebug() << "XMPP" << prefix.toAscii().constData() << message.c_str();
}

void Connection::RefreshPeers() {
  // Clear the lists
  d->peers_.clear();
  d->querying_peers_.clear();

  // Query presence
  qDebug() << "Sending presence query";
  d->client_->send(gloox::Stanza::createPresenceStanza(d->client_->jid().bareJID()));
}

Connection::Peer::Peer()
    : caps_(0) {
}

void Connection::Private::handleRosterPresence(
    const gloox::RosterItem& item, const std::string& res,
    gloox::Presence presence, const std::string&) {
  // Ignore presence from anyone else
  if (item.jid() != client_->jid().bare()) {
    return;
  }

  QString resource = QString::fromUtf8(res.c_str());

  switch (presence) {
  case gloox::PresenceUnknown:
  case gloox::PresenceUnavailable:
    // The peer went offline - did we know about him?
    qDebug() << "Peer unavailable" << resource;

    for (int i=0 ; i<querying_peers_.count() ; ++i) {
      if (querying_peers_[i].jid_resource_ == resource) {
        querying_peers_.takeAt(i);
        break;
      }
    }

    for (int i=0 ; i<peers_.count() ; ++i) {
      if (peers_[i].jid_resource_ == resource) {
        emit parent_->PeerRemoved(peers_.takeAt(i));
        break;
      }
    }
    break;

  default:
    // The peer came online
    if (!has_peer(resource)) {
      qDebug() << "Got presence from" << resource;

      // This is a peer on our own bare JID, and we haven't seen it before
      gloox::JID full_jid(item.jid());
      full_jid.setResource(res);

      Peer peer;
      peer.jid_resource_ = resource;
      querying_peers_ << peer;

      client_->disco()->getDiscoInfo(full_jid, std::string(), this, 0);
    }
  }
}

bool Connection::Private::has_peer(const QString& jid_resource) const {
  foreach (const Peer& peer, peers_) {
    if (peer.jid_resource_ == jid_resource) {
      return true;
    }
  }

  foreach (const Peer& peer, querying_peers_) {
    if (peer.jid_resource_ == jid_resource) {
      return true;
    }
  }
}

void Connection::Private::handleDiscoInfoResult(gloox::Stanza* stanza, int context) {
  // Is this from our own bare JID?
  if (stanza->from().bareJID() != client_->jid().bareJID()) {
    return;
  }

  QString resource = QString::fromUtf8(stanza->from().resource().c_str());

  qDebug() << "Got disco info from" << resource;

  // Are we currently querying this peer?
  int querying_peer_index = -1;
  for (int i=0 ; i<querying_peers_.count() ; ++i) {
    if (querying_peers_[i].jid_resource_ == resource) {
      querying_peer_index = i;
      break;
    }
  }
  if (querying_peer_index == -1) {
    return;
  }

  // Remove this peer from the querying list and try to fill in his info.
  Peer peer = querying_peers_.takeAt(querying_peer_index);

  // Check for requried tags in the stanza.
  gloox::Tag* query = stanza->findChild("query");
  if (!query) {
    return;
  }

  gloox::Tag* identity = query->findChild("identity");
  gloox::Tag::TagList features = query->findChildren("feature");
  if (identity == NULL || features.size() == 0) {
    return;
  }

  // Fill in the name.
  peer.agent_name_ = QString::fromUtf8(identity->findAttribute("name").c_str());

  // Fill in the list of capabilities.
  foreach (gloox::Tag* feature, features) {
    const std::string feature_name = feature->findAttribute("var");
    if (feature_name == kXmlnsXrmeMediaPlayer) {
      peer.caps_ |= Peer::MediaPlayer;
    }
    if (feature_name == kXmlnsXrmeRemoteControl) {
      peer.caps_ |= Peer::RemoteControl;
    }
  }

  // No recognised capabilities?  Discard the peer.
  if (peer.caps_ == Peer::None) {
    return;
  }

  peers_ << peer;
  emit parent_->PeerFound(peer);

  qDebug() << "Peer found:" << peer.agent_name_ << peer.jid_resource_ << peer.caps_;
}

void Connection::Private::handleDiscoError(gloox::Stanza* stanza, int context) {
  QString resource = QString::fromUtf8(stanza->from().resource().c_str());

  // Remove this peer if we're currently querying it
  for (int i=0 ; i<querying_peers_.count() ; ++i) {
    if (querying_peers_[i].jid_resource_ == resource) {
      querying_peers_.removeAt(i);
      return;
    }
  }
}

} // namespace xrme
