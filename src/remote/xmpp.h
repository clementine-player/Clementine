#ifndef XMPP_H
#define XMPP_H

#include <gloox/client.h>
#include <gloox/messagehandler.h>

#include <boost/scoped_ptr.hpp>

#include <QSocketNotifier>
#include <QString>

class XMPP : public QObject, public gloox::MessageHandler {
  Q_OBJECT
 public:
  XMPP();
  virtual ~XMPP();

  bool Connect(const QString& jid, const QString& password);

 private slots:
  void Receive();

 private:
  // gloox::MessageHandler
  virtual void handleMessage(const gloox::Message& stanza,
                             gloox::MessageSession* session = 0);

  boost::scoped_ptr<gloox::Client> client_;
  boost::scoped_ptr<QSocketNotifier> notifier_;
};

#endif  // XMPP_H
