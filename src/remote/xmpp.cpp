#include "xmpp.h"

#include <gloox/connectiontcpclient.h>
#include <gloox/message.h>

#include <QSettings>
#include <QtDebug>

#include "keychain.h"

using gloox::Client;
using gloox::ConnectionTCPClient;
using gloox::JID;
using gloox::Message;
using gloox::MessageSession;

XMPP::XMPP() {

}

XMPP::~XMPP() {

}

void XMPP::Connect() {
  QSettings s;
  s.beginGroup("remote");
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
  client_->registerConnectionListener(this);
  client_->registerMessageHandler(this);
  client_->setServer("talk.google.com");
  client_->connect(false);
  int fd = static_cast<ConnectionTCPClient*>(client_->connectionImpl())->socket();

  notifier_.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
  connect(notifier_.get(), SIGNAL(activated(int)), SLOT(Receive()));
}

void XMPP::handleMessage(const Message& stanza, MessageSession* session) {
  qDebug() << Q_FUNC_INFO;
  qDebug() << stanza.tag()->xml().c_str();
  Message reply(Message::Chat, stanza.from(), "Hello World!");
  client_->send(reply);
}

void XMPP::onConnect() {
  qDebug() << "Connected with resource:" << client_->resource().c_str();
}

void XMPP::onDisconnect(gloox::ConnectionError e) {
  qDebug() << "Disconnected:" << e;
}

bool XMPP::onTLSConnect(const gloox::CertInfo& info) {
  return true;
}

void XMPP::Receive() {
  client_->recv();
}
