#include "../client.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../message.h"
#include "../gloox.h"
#include "../loghandler.h"
#include "../tlshandler.h"
#include "../tlsdefault.h"
#include "../logsink.h"
#include "../messagehandler.h"
#include "../base64.h"
using namespace gloox;

#include <unistd.h>
#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

#ifdef WIN32
#include <windows.h>
#endif

#include "../config.h"

#ifdef HAVE_GNUTLS

/*
 * Using TLS to encrypt end-to-end traffic is not a recommended practice in XMPP,
 * nor is it standardized in any way. Use this code at your own risk.
 */

class MessageTest : public ConnectionListener, LogHandler,
                    MessageHandler, TLSHandler
{
  public:
    MessageTest()
      : m_tls( new TLSDefault( this, "", TLSDefault::AnonymousClient ) ),
        rcpt( "hurkhurk@example.net/server" ) {}

    virtual ~MessageTest()
    {
      delete m_tls;
    }

    void start()
    {

      JID jid( "hurkhurk@example.net/client" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->registerMessageHandler( this );
      j->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      j->disco()->addFeature( XMLNS_CHAT_STATES );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      j->connect();

      delete j;
    }

    virtual void onConnect()
    {
      printf( "connected!\n" );
      m_tls->handshake();
      xtlsSend();
      m_send = "";
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

    void xtlsSend()
    {
      Tag *m = new Tag( "message" );
      m->addAttribute( "to", rcpt.full() );
      m->addAttribute( "type", "chat" );
      Tag *x = new Tag( m, "xtls", Base64::encode64( m_send ) );
      x->addAttribute( "xmlns", "test:xtls" );
      m_send = "";
      j->send( m );
    }

    virtual void handleEncryptedData( const TLSBase* /*base*/, const std::string& data )
    {
      printf( "caching %d bytes of encrypted data\n", data.length() );
      m_send += data;
    }

    virtual void handleDecryptedData( const TLSBase* /*base*/, const std::string& data )
    {
      printf( "decrypted packet contents: %s\n", data.c_str() );
      m_tls->encrypt( "bye" );
      xtlsSend();
      j->disconnect();
    }

    virtual void handleHandshakeResult( const TLSBase* /*base*/, bool success, CertInfo& /*certinfo*/ )
    {
      if( success )
      {
        printf( "xtls handshake successful!\n" );
        m_tls->encrypt( "ping" );
        xtlsSend();
      }
      else
      {
        printf( "xtls handshake failed!\n" );
        j->disconnect();
      }
    }

    virtual void handleMessage( const Message& msg, MessageSession * /*session*/ )
    {
      Tag* m = msg.tag();
      Tag *x = m->findChild( "xtls", "xmlns", "test:xtls" );
      if( x )
      {
        printf( "decrypting: %d\n", x->cdata().length() );
        m_tls->decrypt( Base64::decode64( x->cdata() ) );
        xtlsSend();
      }
      delete m;
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

  private:
    Client *j;
    TLSBase* m_tls;
    std::string m_send;
    const JID rcpt;
};
#endif // HAVE_GNUTLS

int main( int /*argc*/, char** /*argv*/ )
{
#ifdef HAVE_GNUTLS
  MessageTest *r = new MessageTest();
  r->start();
  delete( r );
#endif // HAVE_GNUTLS
  return 0;
}
