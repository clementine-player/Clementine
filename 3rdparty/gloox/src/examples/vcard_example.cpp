
#include "../client.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../stanza.h"
#include "../gloox.h"
#include "../loghandler.h"
#include "../vcardhandler.h"
#include "../vcardmanager.h"
#include "../vcard.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

class VCardTest : public ConnectionListener, LogHandler, VCardHandler
{
  public:
    VCardTest() : m_count( 0 ) {}
    virtual ~VCardTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->disableRoster();
      j->registerConnectionListener( this );
      j->disco()->setVersion( "discoTest", GLOOX_VERSION, "linux" );
      j->disco()->setIdentity( "client", "bot" );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );
//       j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
      m_vManager = new VCardManager( j );

      j->connect();

      delete( m_vManager );
      delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected\n" );
      JID jid( "hurkhurk@example.org" );
      m_vManager->fetchVCard( jid, this );
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

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleVCard( const JID& jid, const VCard *v )
    {
      ++m_count;
      if( !v )
      {
        printf( "empty vcard!\n" );
        return;
      }

      VCard* vcard = new VCard( *v );
      printf( "received vcard for %s: %s, %d\n", jid.full().c_str(), vcard->tag()->xml().c_str(), m_count );
      VCard::AddressList::const_iterator it = vcard->addresses().begin();
      for( ; it != vcard->addresses().end(); ++it )
      {
        printf( "address: %s\n", (*it).street.c_str() );
      }

      if( m_count > 2 )
        j->disconnect();
      else if( m_count == 1 )
      {
        VCard *v = new VCard();
        v->setFormattedname( "Hurk the Hurk" );
        v->setNickname( "hurkhurk" );
        v->setName( "Simpson", "Bart", "", "Mr.", "jr." );
        v->addAddress( "pobox", "app. 2", "street", "Springfield", "region", "123", "USA", VCard::AddrTypeHome );
        m_vManager->storeVCard( v, this );
        printf( "setting vcard: %s\n", v->tag()->xml().c_str() );
      }
      else
      {
        JID jid( "hurkhurk@example.org" );
        m_vManager->fetchVCard( jid, this );
      }
    }

    virtual void handleVCardResult( VCardContext context, const JID& jid,
                                    StanzaError se = StanzaErrorUndefined  )
    {
      printf( "vcard result: context: %d, jid: %s, error: %d\n", context, jid.full().c_str(), se );
      m_vManager->fetchVCard( jid, this );
    }

  private:
    Client *j;
    VCardManager *m_vManager;
    int m_count;
};

int main( int /*argc*/, char** /*argv*/ )
{
  VCardTest *v = new VCardTest();
  v->start();
  delete( v );
  return 0;
}
