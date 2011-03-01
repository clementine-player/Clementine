#include "../client.h"
#include "../messagehandler.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../message.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../flexoff.h"
#include "../flexoffhandler.h"
#include "../loghandler.h"
#include "../logsink.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

class FlexOffTest : public MessageHandler, ConnectionListener, FlexibleOfflineHandler,
                           LogHandler
{
  public:
    FlexOffTest() {}
    virtual ~FlexOffTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->registerMessageHandler( this );
      j->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      f = new FlexibleOffline( j );
      f->registerFlexibleOfflineHandler( this );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      j->connect();

      delete( j );
    }

    virtual void onConnect()
    {
      f->checkSupport();
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "message_test: disconnected: %d\n", e );
      if( e == ConnAuthenticationFailed )
        printf( "auth failed. reason: %d\n", j->authError() );
    }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void handleMessage( const Message& msg, MessageSession * /*session*/ )
    {
      printf( "type: %d, subject: %s, message: %s, thread id: %s\n", msg.subtype(),
              msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str() );
      Tag *m = new Tag( "message" );
      m->addAttribute( "from", j->jid().full() );
      m->addAttribute( "to", msg.from().full() );
      m->addAttribute( "type", "chat" );
      Tag *b = new Tag( "body", "You said:\n> " + msg.body() + "\nI like that statement." );
      m->addChild( b );
      if( !msg.subject().empty() )
      {
        Tag *s = new Tag( "subject", "Re:" +  msg.subject() );
        m->addChild( s );
      }
      j->send( m );
    }

    virtual void handleFlexibleOfflineSupport( bool support )
    {
      if( support )
      {
        printf( "FlexOff: supported\n" );
        f->getMsgCount();
      }
      else
      {
        printf( "FlexOff: not supported\n" );
        j->disconnect();
      }
    }

    virtual void handleFlexibleOfflineMsgNum( int num )
    {
      printf( "FlexOff messgaes: %d\n", num );
      f->fetchHeaders();
    }

    virtual void handleFlexibleOfflineMessageHeaders( const Disco::ItemList& headers )
    {
      printf( "FlexOff: %d headers received.\n", headers.size() );
      StringList l;
      l.push_back( "Fdd" );
      l.push_back( (*(headers.begin()))->node() );
      f->fetchMessages( l );
      f->removeMessages( l );
    }

    virtual void handleFlexibleOfflineResult( FlexibleOfflineResult result )
    {
      printf( "FlexOff: result: %d\n", result );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

  private:
    Client *j;
    FlexibleOffline *f;
};

int main( int /*argc*/, char** /*argv*/ )
{
  FlexOffTest *r = new FlexOffTest();
  r->start();
  delete( r );
  return 0;
}
