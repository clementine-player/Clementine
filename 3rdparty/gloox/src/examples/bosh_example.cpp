#include "../client.h"
#include "../messagesessionhandler.h"
#include "../messageeventhandler.h"
#include "../messageeventfilter.h"
#include "../chatstatehandler.h"
#include "../chatstatefilter.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../message.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../connectiontcpclient.h"
#include "../connectionsocks5proxy.h"
#include "../messagehandler.h"
#include "../connectionbosh.h"
#include "../connectionhttpproxy.h"

using namespace gloox;

#include <unistd.h>
#include <stdio.h>
#include <string>

#include <cstdio> // [s]print[f]

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

class MessageTest : public MessageSessionHandler, ConnectionListener, LogHandler,
                    MessageEventHandler, MessageHandler, ChatStateHandler
{
  public:
    MessageTest() : m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ) {}

    virtual ~MessageTest() {}

    void start()
    {

      JID jid( "js@example.net/bosh" );
      j = new Client( jid, "test" );
      j->registerConnectionListener( this );
      j->registerMessageSessionHandler( this, 0 );
      j->disco()->setIdentity( "client", "bot" );
      j->disco()->addFeature( XMLNS_CHAT_STATES );
      j->setCompression(false);
      //StringList ca;
      //ca.push_back( "/path/to/cacert.crt" );
      //j->setCACerts( ca );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );


/*
      ConnectionTCPClient* conn0 = new ConnectionTCPClient( j->logInstance(), "example.net", 8081 );
      ConnectionHTTPProxy* conn1 = new ConnectionHTTPProxy( conn0, j->logInstance(), "foo", 8080 );
      ConnectionBOSH* conn2 = new ConnectionBOSH( j, conn1, j->logInstance(), "camaya.net", "camaya.net" );
      conn2->setMode( ConnectionBOSH::ModeLegacyHTTP );
//        conn2->setMode( ConnectionBOSH::ModePersistentHTTP );
      j->setConnectionImpl( conn2 );
*/

      // this code connects to a jabber server through a BOSH connection...
      ConnectionTCPClient* conn0 = new ConnectionTCPClient( j->logInstance(), "example.net", 8080 );
      ConnectionBOSH* conn1 = new ConnectionBOSH( j, conn0, j->logInstance(), "example.net", "example.net" );
//       conn1->setMode( ConnectionBOSH::ModeLegacyHTTP );
//        conn1->setMode( ConnectionBOSH::ModePersistentHTTP );
      j->setConnectionImpl( conn1 );

      j->setForceNonSasl( true );




      if( j->connect( false ) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          ce = j->recv();
        }
        printf( "ce: %d\n", ce );
      }

      delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "message_test: disconnected: %d\n", e );
      if( e == ConnAuthenticationFailed )
        printf( "auth failed. reason: %d\n", j->authError() );
    }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      time_t from( info.date_from );
      time_t to( info.date_to );

      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
              "from: %s\nto: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str(), ctime( &from ), ctime( &to ) );
      return true;
    }

    virtual void handleMessage( const Message& msg, MessageSession * /*session*/ )
    {
      printf( "type: %d, subject: %s, message: %s, thread id: %s\n", msg.subtype(),
              msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str() );

      std::string re = "You said:\n> " + msg.body() + "\nI like that statement.";
      std::string sub;
      if( !msg.subject().empty() )
        sub = "Re: " +  msg.subject();

      m_messageEventFilter->raiseMessageEvent( MessageEventDisplayed );
      m_messageEventFilter->raiseMessageEvent( MessageEventComposing );
      m_chatStateFilter->setChatState( ChatStateComposing );
      m_session->send( re, sub );

      if( msg.body() == "quit" )
        j->disconnect();
    }

    virtual void handleMessageEvent( const JID& from, MessageEventType event )
    {
      printf( "received event: %d from: %s\n", event, from.full().c_str() );
    }

    virtual void handleChatState( const JID& from, ChatStateType state )
    {
      printf( "received state: %d from: %s\n", state, from.full().c_str() );
    }

    virtual void handleMessageSession( MessageSession *session )
    {
      printf( "got new session\n");
      // this example can handle only one session. so we get rid of the old session
      j->disposeMessageSession( m_session );
      m_session = session;
      m_session->registerMessageHandler( this );
      m_messageEventFilter = new MessageEventFilter( m_session );
      m_messageEventFilter->registerMessageEventHandler( this );
      m_chatStateFilter = new ChatStateFilter( m_session );
      m_chatStateFilter->registerChatStateHandler( this );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
	printf("%d: ", int( time( 0 ) ) );
	switch(area)
	{
          case LogAreaXmlIncoming:
           printf("Received XML: ");
	   break;
	  case LogAreaXmlOutgoing:
	   printf("Sent XML: ");
	   break;
	  case LogAreaClassConnectionBOSH:
	   printf("BOSH: ");
	   break;
	  default:
           printf("log: level: %d, area: %d, ", level, area);
	}
      printf("%s\n", message.c_str() );
    }

  private:
    Client *j;
    MessageSession *m_session;
    MessageEventFilter *m_messageEventFilter;
    ChatStateFilter *m_chatStateFilter;
};

int main( int /*argc*/, char** /*argv*/ )
{
  MessageTest *r = new MessageTest();
  r->start();
  delete( r );
  return 0;
}
