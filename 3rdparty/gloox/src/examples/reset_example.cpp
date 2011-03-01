#include "../client.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../rostermanager.h"
#include "../loghandler.h"
#include "../logsink.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

class RosterTest : public ConnectionListener, LogHandler
{
  public:
    RosterTest() {}
    virtual ~RosterTest() {}

    void start()
    {

      JID jid( "hurkhurkss@example.net/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->disco()->setVersion( "resetTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      printf( "first run: %d\n", j->connect() );
      printf( "second run: %d\n", j->connect() );

      delete( j );
    }

    virtual void onConnect()
    {
      j->disconnect();
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "reset_test: disconnected: %d\n", e );
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
