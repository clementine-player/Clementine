#include "../client.h"
#include "../connectionlistener.h"
#include "../privacylisthandler.h"
#include "../disco.h"
#include "../privacymanager.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

class PLTest : public PrivacyListHandler, ConnectionListener
{
  public:
    PLTest() {}
    virtual ~PLTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );

      j->registerConnectionListener(this );
      j->disco()->setVersion( "PLTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );

      p = new PrivacyManager( j );
      p->registerPrivacyListHandler( this );

      j->connect();

      delete( p );
      delete( j );
    }

    virtual void onConnect()
    {
      p->requestListNames();
    }

    virtual void onDisconnect( ConnectionError /*e*/ ) { printf( "disco_test: disconnected\n" ); }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void handlePrivacyListNames( const std::string& active, const std::string& def,
                                         const StringList& lists )
    {
      printf( "received PL...\n" );
      printf( "active list: %s\n", active.c_str() );
      printf( "default list: %s\n", def.c_str() );
      printf( "all lists:\n" );
      StringList::const_iterator it = lists.begin();
      for( ; it != lists.end(); it++ )
      {
        printf( "%s\n", (*it).c_str() );
      }

      PrivacyListHandler::PrivacyList l;
      PrivacyItem item( PrivacyItem::TypeJid, PrivacyItem::ActionDeny,
                        PrivacyItem::PacketMessage, "me@there.com" );
      l.push_back( item );
      PrivacyItem item2( PrivacyItem::TypeJid, PrivacyItem::ActionAllow,
                        PrivacyItem::PacketIq, "me@example.org" );
      l.push_back( item2 );
      p->store( "mnyList", l );
//       p->unsetDefault();
//       p->unsetActive();
      p->requestList( "mnyList" );
    }

    virtual void handlePrivacyListResult( const std::string& id, PrivacyListResult plResult )
    {
      printf( "result for id '%s': %d\n", id.c_str(), plResult );
    }

    virtual void handlePrivacyList( const std::string& name, const PrivacyList& items )
    {
      printf( "received list: %s\n", name.c_str() );
      PrivacyListHandler::PrivacyList::const_iterator it = items.begin();
      for( ; it != items.end(); it++ )
      {
        printf( "item: type: %d, action: %d, packetType: %d, value: %s\n",
                (*it).type(), (*it).action(), (*it).packetType(), (*it).value().c_str() );
      }
    }

    virtual void handlePrivacyListChanged( const std::string& name )
    {
      printf( "list changed: %s\n", name.c_str() );
    }

  private:
    Client *j;
    PrivacyManager *p;
};

int main( int /*argc*/, char** /*argv*/ )
{
  PLTest *t = new PLTest();
  t->start();
  delete( t );
  return 0;
}
