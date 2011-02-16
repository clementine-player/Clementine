#include "xmpp.h"

#include <gloox/connectiontcpclient.h>
#include <gloox/message.h>

#include <QtDebug>

using gloox::Client;
using gloox::ConnectionTCPClient;
using gloox::JID;
using gloox::Message;
using gloox::MessageSession;

XMPP::XMPP() {

}

XMPP::~XMPP() {

}

bool XMPP::Connect(const QString& jid, const QString& password) {
  qDebug() << Q_FUNC_INFO;
  // TODO: Generate <256 char resource.
  JID j(jid.toUtf8().constData());
  qDebug() << "Resource:" << j.resource().c_str();
  client_.reset(new Client(j, password.toUtf8().constData()));
  client_->registerMessageHandler(this);
  client_->setServer("talk.google.com");
  client_->connect(false);
  qDebug() << Q_FUNC_INFO;
  int fd = static_cast<ConnectionTCPClient*>(client_->connectionImpl())->socket();

  notifier_.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
  connect(notifier_.get(), SIGNAL(activated(int)), SLOT(Receive()));

  qDebug() << Q_FUNC_INFO;
  return true;
}

void XMPP::handleMessage(const Message& stanza, MessageSession* session) {
  qDebug() << Q_FUNC_INFO;
  qDebug() << stanza.tag()->xml().c_str();
  qDebug() << "resource:" << client_->resource().c_str();
  Message reply(Message::Chat, stanza.from(), "Hello World!");
  client_->send(reply);
}

void XMPP::Receive() {
  qDebug() << Q_FUNC_INFO;
  client_->recv();
}
