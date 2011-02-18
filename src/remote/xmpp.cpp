#include "xmpp.h"

#include <gloox/connectiontcpclient.h>

#include <QSettings>
#include <QtDebug>

#include "keychain.h"
#include "remoteconfig.h"

using gloox::Client;
using gloox::ConnectionTCPClient;
using gloox::JID;
using gloox::MessageSession;

XMPP::XMPP() {

}

XMPP::~XMPP() {

}

void XMPP::Connect() {
  QSettings s;
  s.beginGroup(RemoteConfig::kSettingsGroup);
  QVariant username = s.value("username");
  if (username.isValid()) {
    Keychain* keychain = Keychain::getDefault();
    QString password = keychain->getPassword(username.toString());
    Connect(username.toString() + "/clementine", password);
  } else {
    qWarning() << "No username or password set.";
  }
}

void XMPP::Connect(const QString& jid, const QString& password) {
  // TODO: Generate <256 char resource.
  JID j(jid.toUtf8().constData());

  client_.reset(new Client(j, password.toUtf8().constData()));
  ConnectionTCPClient* connection = new ConnectionTCPClient(
        client_.get(), client_->logInstance(), "talk.google.com");
  client_->setConnectionImpl(connection);

  client_->registerConnectionListener(this);
  client_->registerMessageHandler(this);
  client_->logInstance().registerLogHandler(gloox::LogLevelDebug, gloox::LogAreaAll, this);
  client_->setPresence(gloox::PresenceAvailable, -128);
  client_->connect(false);

  notifier_.reset(new QSocketNotifier(connection->socket(), QSocketNotifier::Read));
  connect(notifier_.get(), SIGNAL(activated(int)), SLOT(Receive()));
}

void XMPP::handleMessage(gloox::Stanza* stanza, MessageSession* session) {
  qDebug() << Q_FUNC_INFO;
  qDebug() << stanza->xml().c_str();
  gloox::Stanza* reply = gloox::Stanza::createMessageStanza(
        stanza->from(), "Hello world!");
  client_->send(reply);
}

void XMPP::onConnect() {
  qDebug() << "Connected with resource:" << client_->resource().c_str()
           << client_->jid().full().c_str();
  client_->login();
}

void XMPP::onDisconnect(gloox::ConnectionError e) {
  qDebug() << "Disconnected:" << e;
  notifier_->setEnabled(false);
}

bool XMPP::onTLSConnect(const gloox::CertInfo& info) {
  return true;
}

void XMPP::Receive() {
  client_->recv();
}

void XMPP::handleLog(gloox::LogLevel level, gloox::LogArea area,
                     const std::string& message) {
  QString prefix = "---";
  if (area == gloox::LogAreaXmlIncoming) {
    prefix = "<<<";
  } else if (area == gloox::LogAreaXmlOutgoing) {
    prefix = ">>>";
  }

  qDebug() << "XMPP" << prefix.toAscii().constData() << message.c_str();
}
