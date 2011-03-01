#include "../client.h"
#include "../connectionlistener.h"
#include "../mucroomhandler.h"
#include "../mucroom.h"
#include "../disco.h"
#include "../presence.h"
#include "../message.h"
#include "../dataform.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../loghandler.h"
#include "../logsink.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

#ifdef WIN32
#include <windows.h>
#endif

class MessageTest : public ConnectionListener, LogHandler, MUCRoomHandler
{
  public:
    MessageTest() {}

    virtual ~MessageTest() {}

    void start()
    {
      JID jid( "hurkhurk@example.net/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->setPresence( Presence::Available, -1 );
      j->disco()->setVersion( "gloox muc_example", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      j->setCompression( false );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      JID nick( "test@conference.jabber.org/glooxmuctest" );
      m_room = new MUCRoom( j, nick, this, 0 );

      if( j->connect( false ) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          ce = j->recv();
        }
        printf( "ce: %d\n", ce );
      }

      // cleanup
      delete m_room;
      delete j;
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
      m_room->join();
      m_room->getRoomInfo();
      m_room->getRoomItems();
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

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleMUCParticipantPresence( MUCRoom * /*room*/, const MUCRoomParticipant participant,
                                            const Presence& presence )
    {
      if( presence.presence() == Presence::Available )
        printf( "!!!!!!!!!!!!!!!! %s is in the room, too\n", participant.nick->resource().c_str() );
      else if( presence.presence() == Presence::Unavailable )
        printf( "!!!!!!!!!!!!!!!! %s left the room\n", participant.nick->resource().c_str() );
      else
        printf( "Presence is %d of %s\n", presence.presence(), participant.nick->resource().c_str() );
    }

    virtual void handleMUCMessage( MUCRoom* /*room*/, const Message& msg, bool priv )
    {
      printf( "%s said: '%s' (history: %s, private: %s)\n", msg.from().resource().c_str(), msg.body().c_str(),
              msg.when() ? "yes" : "no", priv ? "yes" : "no" );
    }

    virtual void handleMUCSubject( MUCRoom * /*room*/, const std::string& nick, const std::string& subject )
    {
      if( nick.empty() )
        printf( "Subject: %s\n", subject.c_str() );
      else
        printf( "%s has set the subject to: '%s'\n", nick.c_str(), subject.c_str() );
    }

    virtual void handleMUCError( MUCRoom * /*room*/, StanzaError error )
    {
      printf( "!!!!!!!!got an error: %d", error );
    }

    virtual void handleMUCInfo( MUCRoom * /*room*/, int features, const std::string& name,
                                    const DataForm* infoForm )
    {
      printf( "features: %d, name: %s, form xml: %s\n",
              features, name.c_str(), infoForm->tag()->xml().c_str() );
    }

    virtual void handleMUCItems( MUCRoom * /*room*/, const Disco::ItemList& items )
    {
      Disco::ItemList::const_iterator it = items.begin();
      for( ; it != items.end(); ++it )
      {
        printf( "%s -- %s is an item here\n", (*it)->jid().full().c_str(), (*it)->name().c_str() );
      }
    }

    virtual void handleMUCInviteDecline( MUCRoom * /*room*/, const JID& invitee, const std::string& reason )
    {
      printf( "Invitee %s declined invitation. reason given: %s\n", invitee.full().c_str(), reason.c_str() );
    }

    virtual bool handleMUCRoomCreation( MUCRoom *room )
    {
      printf( "room %s didn't exist, beeing created.\n", room->name().c_str() );
      return true;
    }

  private:
    Client *j;
    MUCRoom *m_room;
};

int main( int /*argc*/, char** /*argv*/ )
{
  MessageTest *r = new MessageTest();
  r->start();
  delete( r );
  return 0;
}
