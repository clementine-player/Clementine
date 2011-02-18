#ifndef XMPP_H
#define XMPP_H

#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/loghandler.h>
#include <gloox/messagehandler.h>

#include <boost/scoped_ptr.hpp>

#include <QSocketNotifier>
#include <QString>

class XMPP : public QObject,
             public gloox::ConnectionListener,
             public gloox::MessageHandler,
             public gloox::LogHandler {
  Q_OBJECT
 public:
  XMPP();
  virtual ~XMPP();

  void Connect();
  void Connect(const QString& jid, const QString& password);

 private slots:
  void Receive();

 private:
  // gloox::MessageHandler
  virtual void handleMessage(gloox::Stanza* stanza,
                             gloox::MessageSession* session = 0);

  // gloox::ConnectionListener
  virtual void onConnect();
  virtual void onDisconnect(gloox::ConnectionError e);
  virtual bool onTLSConnect(const gloox::CertInfo& info);

  // gloox::LogHandler
  virtual void handleLog(gloox::LogLevel level, gloox::LogArea area,
                         const std::string& message);

  boost::scoped_ptr<gloox::Client> client_;
  boost::scoped_ptr<QSocketNotifier> notifier_;
};

#endif  // XMPP_H
