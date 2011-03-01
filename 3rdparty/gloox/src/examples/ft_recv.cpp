#include "../client.h"
#include "../connectionlistener.h"
#include "../stanza.h"
#include "../gloox.h"
#include "../disco.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../siprofileft.h"
#include "../siprofilefthandler.h"
#include "../bytestreamdatahandler.h"
using namespace gloox;

#include <unistd.h>
#include <stdio.h>
#include <string>

#include <cstdio> // [s]print[f]

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

/**
 * Receives one file and displayes it. Does not save anything.
 */
class FTTest : public LogHandler, ConnectionListener, SIProfileFTHandler, BytestreamDataHandler
{
  public:
    FTTest() : m_quit( false ) {}

    virtual ~FTTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.net/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->disco()->setVersion( "ftTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      f = new SIProfileFT( j, this );
      // you should obtain this using disco, really
      f->addStreamHost( JID( "proxy.jabber.org" ), "208.245.212.98", 7777 );

      if( j->connect( false ) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          if( m_quit )
            j->disconnect();

          ce = j->recv( 100 );
          std::list<Bytestream*>::iterator it = m_bs.begin();
          for( ; it != m_bs.end(); ++it )
            (*it)->recv( 100 );
        }
        printf( "ce: %d\n", ce );
      }

      f->dispose( m_bs.front() );
      delete f;
      delete j;
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

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleFTRequest( const JID& from, const JID& /*to*/, const std::string& sid,
                                  const std::string& name, long size, const std::string& hash,
                                  const std::string& date, const std::string& mimetype,
                                  const std::string& desc, int /*stypes*/ )
    {
      printf( "received ft request from %s: %s (%ld bytes, sid: %s). hash: %s, date: %s, mime-type: %s\n"
              "desc: %s\n",
              from.full().c_str(), name.c_str(), size, sid.c_str(), hash.c_str(), date.c_str(),
              mimetype.c_str(), desc.c_str() );
      f->acceptFT( from, sid, SIProfileFT::FTTypeIBB );
    }

//     virtual void handleFTRequestResult( const JID& /*from*/, const std::string& /*sid*/ )
//     {
//     }

    virtual void handleFTRequestError( const IQ& /*iq*/, const std::string& /*sid*/ )
    {
      printf( "ft request error\n" );
    }

    virtual void handleFTBytestream( Bytestream* bs )
    {
      printf( "received bytestream of type: %s", bs->type() == Bytestream::S5B ? "s5b" : "ibb" );
      m_bs.push_back( bs );
      bs->registerBytestreamDataHandler( this );
      if( bs->connect() )
      {
        if( bs->type() == Bytestream::S5B )
          printf( "ok! s5b connected to streamhost\n" );
        else
          printf( "ok! ibb sent request to remote entity\n" );
      }
    }

    virtual const std::string handleOOBRequestResult( const JID& /*from*/, const JID& /*to*/, const std::string& /*sid*/ )
    {
      return std::string();
    };

    virtual void handleBytestreamData( Bytestream* /*s5b*/, const std::string& data )
    {
      printf( "received %d bytes of data:\n%s\n", data.length(), data.c_str() );
    }

    virtual void handleBytestreamError( Bytestream* /*s5b*/, const IQ& /*stanza*/ )
    {
      printf( "socks5 stream error\n" );
    }

    virtual void handleBytestreamOpen( Bytestream* /*s5b*/ )
    {
      printf( "stream opened\n" );
    }

    virtual void handleBytestreamClose( Bytestream* /*s5b*/ )
    {
      printf( "stream closed\n" );
      m_quit = true;
    }

  private:
    Client *j;
    SIProfileFT* f;
    SOCKS5BytestreamManager* s5b;
    std::list<Bytestream*> m_bs;
    bool m_quit;
};

int main( int /*argc*/, char** /*argv*/ )
{
  FTTest *r = new FTTest();
  r->start();
  delete( r );
  return 0;
}
