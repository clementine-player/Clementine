#include "../client.h"
#include "../connectionlistener.h"
#include "../adhoccommandprovider.h"
#include "../disco.h"
#include "../adhoc.h"
#include "../tag.h"
#include "../loghandler.h"
#include "../logsink.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]


class AdhocTest : public ConnectionListener, AdhocCommandProvider, LogHandler
{
  public:
    AdhocTest() {}
    virtual ~AdhocTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->disableRoster();
      j->registerConnectionListener( this );
      j->disco()->setVersion( "adhocTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );
      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      a = new Adhoc( j );
      a->registerAdhocCommandProvider( this, "helloworld", "Hello World!" );
      a->registerAdhocCommandProvider( this, "config", "Configuration" );
      a->registerAdhocCommandProvider( this, "shutdown", "Shutdown" );

      j->connect();

      delete( j );
    }

    virtual void handleAdhocCommand( const JID& from, const Adhoc::Command& command,
                                     const std::string& /*sess*/ )
    {
      if( command.node() == "helloworld" )
        printf( "Hello World!, by %s\n", from.full().c_str() );
      else if( command.node() == "config" )
        printf( "configuration command called by %s\n", from.full().c_str() );
      else if( command.node() == "shutdown" )
      {
        printf( "shutting down, by %s\n", from.full().c_str() );
      }
    }

    virtual void onConnect()
    {
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

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

  private:
    Client *j;
    Adhoc *a;
};

int main( int /*argc*/, char** /*argv*/ )
{
  AdhocTest *b = new AdhocTest();
  b->start();
  delete( b );
  return 0;
}
