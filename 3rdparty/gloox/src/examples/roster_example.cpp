#include "../client.h"
#include "../connectionlistener.h"
#include "../discohandler.h"
#include "../disco.h"
#include "../rostermanager.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../messagehandler.h"
#include "../message.h"
#include "../presence.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

class RosterTest : public RosterListener, ConnectionListener, LogHandler, MessageHandler
{
  public:
    RosterTest() {}
    virtual ~RosterTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->registerMessageHandler( this );
      j->rosterManager()->registerRosterListener( this );
      j->disco()->setVersion( "rosterTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      j->connect();

      delete( j );
    }

    virtual void onConnect()
    {
    }

    virtual void onDisconnect( ConnectionError e ) { printf( "disco_test: disconnected: %d\n", e ); }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void onResourceBindError( ResourceBindError error )
    {
      printf( "onResourceBindError: %d\n", error );
    }

    virtual void onSessionCreateError( SessionCreateError error )
    {
      printf( "onSessionCreateError: %d\n", error );
    }

    virtual void handleItemSubscribed( const JID& jid )
    {
      printf( "subscribed %s\n", jid.bare().c_str() );
    }

    virtual void handleItemAdded( const JID& jid )
    {
      printf( "added %s\n", jid.bare().c_str() );
    }

    virtual void handleItemUnsubscribed( const JID& jid )
    {
      printf( "unsubscribed %s\n", jid.bare().c_str() );
    }

    virtual void handleItemRemoved( const JID& jid )
    {
      printf( "removed %s\n", jid.bare().c_str() );
    }

    virtual void handleItemUpdated( const JID& jid )
    {
      printf( "updated %s\n", jid.bare().c_str() );
    }

    virtual void handleRoster( const Roster& roster )
    {
      printf( "roster arriving\nitems:\n" );
      Roster::const_iterator it = roster.begin();
      for( ; it != roster.end(); ++it )
      {
        printf( "jid: %s, name: %s, subscription: %d\n",
                (*it).second->jid().c_str(), (*it).second->name().c_str(),
                (*it).second->subscription() );
        StringList g = (*it).second->groups();
        StringList::const_iterator it_g = g.begin();
        for( ; it_g != g.end(); ++it_g )
          printf( "\tgroup: %s\n", (*it_g).c_str() );
        RosterItem::ResourceMap::const_iterator rit = (*it).second->resources().begin();
        for( ; rit != (*it).second->resources().end(); ++rit )
          printf( "resource: %s\n", (*rit).first.c_str() );
      }
    }

    virtual void handleRosterError( const IQ& /*iq*/ )
    {
      printf( "a roster-related error occured\n" );
    }

    virtual void handleRosterPresence( const RosterItem& item, const std::string& resource,
                                       Presence::PresenceType presence, const std::string& /*msg*/ )
    {
      printf( "presence received: %s/%s -- %d\n", item.jid().c_str(), resource.c_str(), presence );
    }

    virtual void handleSelfPresence( const RosterItem& item, const std::string& resource,
                                       Presence::PresenceType presence, const std::string& /*msg*/ )
    {
      printf( "self presence received: %s/%s -- %d\n", item.jid().c_str(), resource.c_str(), presence );
    }

    virtual bool handleSubscriptionRequest( const JID& jid, const std::string& /*msg*/ )
    {
      printf( "subscription: %s\n", jid.bare().c_str() );
      StringList groups;
      JID id( jid );
      j->rosterManager()->subscribe( id, "", groups, "" );
      return true;
    }

    virtual bool handleUnsubscriptionRequest( const JID& jid, const std::string& /*msg*/ )
    {
      printf( "unsubscription: %s\n", jid.bare().c_str() );
      return true;
    }

    virtual void handleNonrosterPresence( const Presence& presence )
    {
      printf( "received presence from entity not in the roster: %s\n", presence.from().full().c_str() );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleMessage( const Message& msg, MessageSession * /*session*/ )
    {
      if( msg.body() == "quit" )
        j->disconnect();
      else if( msg.body() == "subscribe" )
        j->rosterManager()->subscribe( msg.from() );
      else if( msg.body() == "unsubscribe" )
        j->rosterManager()->unsubscribe( msg.from() );
      else if( msg.body() == "cancel" )
        j->rosterManager()->cancel( msg.from() );
      else if( msg.body() == "remove" )
        j->rosterManager()->remove( msg.from() );
      else
        printf( "msg: %s\n", msg.body().c_str() );
    }

  private:
    Client *j;
};

int main( int /*argc*/, char** /*argv*/ )
{
  RosterTest *r = new RosterTest();
  r->start();
  delete( r );
  return 0;
}
