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
#include "../connectionhttpproxy.h"
#include "../messagehandler.h"
#include "../pubsubmanager.h"
#include "../pubsubresulthandler.h"
using namespace gloox;

#include <unistd.h>
#include <stdio.h>
#include <string>

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif
/*
class PubsubExample : public MessageSessionHandler, ConnectionListener, LogHandler,
                    MessageEventHandler, MessageHandler, ChatStateHandler, PubSub::ResultHandler
{
  public:
    PubsubExample() : m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ) {}

    virtual ~PubsubExample() {}

    void start()
    {

      JID jid( "hurkhurk@example.net/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->registerMessageSessionHandler( this, 0 );
      j->disco()->setVersion( "PubsubExample", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      j->disco()->addFeature( XMLNS_CHAT_STATES );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      pubsub = new PubSub::Manager( j );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

//
// this code connects to a jabber server through a SOCKS5 proxy
//
//       ConnectionSOCKS5Proxy* conn = new ConnectionSOCKS5Proxy( j,
//                                   new ConnectionTCP( j->logInstance(),
//                                                      "sockshost", 1080 ),
//                                   j->logInstance(), "example.net" );
//       conn->setProxyAuth( "socksuser", "sockspwd" );
//       j->setConnectionImpl( conn );

//
// this code connects to a jabber server through a HTTP proxy through a SOCKS5 proxy
//
//       ConnectionTCP* conn0 = new ConnectionTCP( j->logInstance(), "old", 1080 );
//       ConnectionSOCKS5Proxy* conn1 = new ConnectionSOCKS5Proxy( conn0, j->logInstance(), "old", 8080 );
//       conn1->setProxyAuth( "socksuser", "sockspwd" );
//       ConnectionHTTPProxy* conn2 = new ConnectionHTTPProxy( j, conn1, j->logInstance(), "jabber.cc" );
//       conn2->setProxyAuth( "httpuser", "httppwd" );
//       j->setConnectionImpl( conn2 );


      j->connect( true );

      delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "PubsubExample: disconnected: %d\n", e );
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

    virtual void handleMessage( Message* msg, MessageSession * session )
    {
      if( msg->body() == "quit" )
        j->disconnect();
      else if( msg->body() == "create" )
        pubsub->createCollectionNode( JID( "pubsub.jabber.ru" ), "blah", this, "blubb", "pubsub/nodes" );
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
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleNodeCreationResult( const JID& service,
                                            const std::string& node,
                                            const Error& e )
    {
      printf( "created node '%s' on '%s'\n", node.c_str(), service.bare().c_str() );
    }

    virtual void handleNodeDeletationResult( const JID& service,
                                              const std::string& node,
                                              const Error& e ) {}

    virtual void handleNodePurgeResult( const JID& service,
                                        const std::string& node,
                                        const Error& e ) {}

    virtual void handleSubscriptionOptions( const JID& service, const JID& jid,
                                            const std::string& node,
                                            const DataForm& options ) {}

    virtual void handleSubscriptionOptionsResult( const JID& service,
                                                  //const JID& jid,
                                                  const std::string& node,
                                                  const Error& e ) {}

    virtual void handleSubscriberList( const JID& service, const std::string& node,
                                        const PubSub::SubscriberList& list ) {}

    virtual void handleSubscriberListResult( const JID& service, const std::string& node,
                                              const Error& e ) {}

    virtual void handleAffiliateList( const JID& service, const std::string& node,
                                      const PubSub::AffiliateList& list ) {}

    virtual void handleAffiliateListResult( const JID& service, const std::string& node,
                                            const Error& e ) {}

    virtual void handleNodeConfig( const JID& service, const std::string& node,
                                    const DataForm& config ) {}

    virtual void handleNodeConfigResult( const JID& service, const std::string& node,
                                          const Error& e ) {}

  private:
    Client *j;
    MessageSession *m_session;
    MessageEventFilter *m_messageEventFilter;
    ChatStateFilter *m_chatStateFilter;
    PubSub::Manager *pubsub;
};
*/
int main( int /*argc*/, char** /*argv*/ )
{
  //PubsubExample *r = new PubsubExample();
  //r->start();
  //delete( r );
  return 0;
}
