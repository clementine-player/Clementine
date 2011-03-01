/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "config.h"

#include "clientbase.h"
#include "connectionbase.h"
#include "tlsbase.h"
#include "compressionbase.h"
#include "connectiontcpclient.h"
#include "disco.h"
#include "messagesessionhandler.h"
#include "tag.h"
#include "iq.h"
#include "message.h"
#include "subscription.h"
#include "presence.h"
#include "connectionlistener.h"
#include "iqhandler.h"
#include "messagehandler.h"
#include "presencehandler.h"
#include "rosterlistener.h"
#include "subscriptionhandler.h"
#include "loghandler.h"
#include "taghandler.h"
#include "mucinvitationhandler.h"
#include "mucroom.h"
#include "jid.h"
#include "base64.h"
#include "error.h"
#include "md5.h"
#include "util.h"
#include "tlsdefault.h"
#include "compressionzlib.h"
#include "stanzaextensionfactory.h"
#include "eventhandler.h"
#include "event.h"

#include <cstdlib>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdio>

#include <string.h> // for memset()

#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
#include <tchar.h>
#endif

namespace gloox
{

  // ---- ClientBase::Ping ----
  ClientBase::Ping::Ping()
    : StanzaExtension( ExtPing )
  {
  }

  ClientBase::Ping::~Ping()
  {
  }

  const std::string& ClientBase::Ping::filterString() const
  {
    static const std::string filter = "/iq/ping[@xmlns='" + XMLNS_XMPP_PING + "']";
    return filter;
  }
  // ---- ~ClientBase::Ping ----

  // ---- ClientBase ----
  ClientBase::ClientBase( const std::string& ns, const std::string& server, int port )
    : m_connection( 0 ), m_encryption( 0 ), m_compression( 0 ), m_disco( 0 ), m_namespace( ns ),
      m_xmllang( "en" ), m_server( server ), m_compressionActive( false ), m_encryptionActive( false ),
      m_compress( true ), m_authed( false ), m_block( false ), m_sasl( true ), m_tls( TLSOptional ), m_port( port ),
      m_availableSaslMechs( SaslMechAll ),
      m_statisticsHandler( 0 ), m_mucInvitationHandler( 0 ),
      m_messageSessionHandlerChat( 0 ), m_messageSessionHandlerGroupchat( 0 ),
      m_messageSessionHandlerHeadline( 0 ), m_messageSessionHandlerNormal( 0 ),
      m_parser( this ), m_seFactory( 0 ), m_authError( AuthErrorUndefined ),
      m_streamError( StreamErrorUndefined ), m_streamErrorAppCondition( 0 ),
      m_selectedSaslMech( SaslMechNone ), m_autoMessageSession( false )
  {
    init();
  }

  ClientBase::ClientBase( const std::string& ns, const std::string& password,
                          const std::string& server, int port )
    : m_connection( 0 ), m_encryption( 0 ), m_compression( 0 ), m_disco( 0 ), m_namespace( ns ),
      m_password( password ),
      m_xmllang( "en" ), m_server( server ), m_compressionActive( false ), m_encryptionActive( false ),
      m_compress( true ), m_authed( false ), m_block( false ), m_sasl( true ), m_tls( TLSOptional ),
      m_port( port ), m_availableSaslMechs( SaslMechAll ),
      m_statisticsHandler( 0 ), m_mucInvitationHandler( 0 ),
      m_messageSessionHandlerChat( 0 ), m_messageSessionHandlerGroupchat( 0 ),
      m_messageSessionHandlerHeadline( 0 ), m_messageSessionHandlerNormal( 0 ),
      m_parser( this ), m_seFactory( 0 ), m_authError( AuthErrorUndefined ),
      m_streamError( StreamErrorUndefined ), m_streamErrorAppCondition( 0 ),
      m_selectedSaslMech( SaslMechNone ), m_autoMessageSession( false )
  {
    init();
  }

  void ClientBase::init()
  {
    if( !m_disco )
    {
      m_disco = new Disco( this );
      m_disco->setVersion( "based on gloox", GLOOX_VERSION );
      m_disco->addFeature( XMLNS_XMPP_PING );
    }

    registerStanzaExtension( new Error() );
    registerStanzaExtension( new Ping() );
    registerIqHandler( this, ExtPing );

    m_streamError = StreamErrorUndefined;
    m_block = false;
    memset( &m_stats, 0, sizeof( m_stats ) );
    cleanup();
  }

  ClientBase::~ClientBase()
  {
    delete m_connection;
    delete m_encryption;
    delete m_compression;
    delete m_seFactory;
    m_seFactory = 0; // to avoid usage when Disco gets deleted below
    delete m_disco;
    m_disco = 0;

    util::clearList( m_messageSessions );

    PresenceJidHandlerList::const_iterator it1 = m_presenceJidHandlers.begin();
    for( ; it1 != m_presenceJidHandlers.end(); ++it1 )
      delete (*it1).jid;
  }

  ConnectionError ClientBase::recv( int timeout )
  {
    if( !m_connection || m_connection->state() == StateDisconnected )
      return ConnNotConnected;

    return m_connection->recv( timeout );
  }

  bool ClientBase::connect( bool block )
  {
    if( m_server.empty() )
      return false;

    if( !m_connection )
      m_connection = new ConnectionTCPClient( this, m_logInstance, m_server, m_port );

    if( m_connection->state() >= StateConnecting )
      return true;

    if( !m_encryption )
      m_encryption = getDefaultEncryption();

    if( !m_compression )
      m_compression = getDefaultCompression();

    m_logInstance.dbg( LogAreaClassClientbase, "This is gloox " + GLOOX_VERSION + ", connecting to "
                                               + m_server + ":" + util::int2string( m_port ) + "..." );
    m_block = block;
    ConnectionError ret = m_connection->connect();
    if( ret != ConnNoError )
      return false;

    if( m_block )
      m_connection->receive();

    return true;
  }

  void ClientBase::handleTag( Tag* tag )
  {
    if( !tag )
    {
      logInstance().dbg( LogAreaClassClientbase, "stream closed" );
      disconnect( ConnStreamClosed );
      return;
    }

    logInstance().dbg( LogAreaXmlIncoming, tag->xml() );
    ++m_stats.totalStanzasReceived;

    if( tag->name() == "stream" && tag->xmlns() == XMLNS_STREAM )
    {
      const std::string& version = tag->findAttribute( "version" );
      if( !checkStreamVersion( version ) )
      {
        logInstance().dbg( LogAreaClassClientbase, "This server is not XMPP-compliant"
            " (it does not send a 'version' attribute). Please fix it or try another one.\n" );
        disconnect( ConnStreamVersionError );
        return;
      }

      m_sid = tag->findAttribute( "id" );
      handleStartNode();
    }
    else if( tag->name() == "error" && tag->xmlns() == XMLNS_STREAM )
    {
      handleStreamError( tag );
      disconnect( ConnStreamError );
    }
    else
    {
      if( !handleNormalNode( tag ) )
      {
        if( tag->xmlns().empty() || tag->xmlns() == XMLNS_CLIENT )
        {
          if( tag->name() == "iq"  )
          {
            IQ iq( tag );
            m_seFactory->addExtensions( iq, tag );
            notifyIqHandlers( iq );
            ++m_stats.iqStanzasReceived;
          }
          else if( tag->name() == "message" )
          {
            Message msg( tag );
            m_seFactory->addExtensions( msg, tag );
            notifyMessageHandlers( msg );
            ++m_stats.messageStanzasReceived;
          }
          else if( tag->name() == "presence" )
          {
            const std::string& type = tag->findAttribute( TYPE );
            if( type == "subscribe"  || type == "unsubscribe"
                || type == "subscribed" || type == "unsubscribed" )
            {
              Subscription sub( tag );
              m_seFactory->addExtensions( sub, tag );
              notifySubscriptionHandlers( sub );
              ++m_stats.s10nStanzasReceived;
            }
            else
            {
              Presence pres( tag );
              m_seFactory->addExtensions( pres, tag );
              notifyPresenceHandlers( pres );
              ++m_stats.presenceStanzasReceived;
            }
          }
          else
            m_logInstance.err( LogAreaClassClientbase, "Received invalid stanza." );
        }
        else
        {
          notifyTagHandlers( tag );
        }
      }
    }

    if( m_statisticsHandler )
      m_statisticsHandler->handleStatistics( getStatistics() );
  }

  void ClientBase::handleCompressedData( const std::string& data )
  {
    if( m_encryption && m_encryptionActive )
      m_encryption->encrypt( data );
    else if( m_connection )
      m_connection->send( data );
    else
      m_logInstance.err( LogAreaClassClientbase, "Compression finished, but chain broken" );
  }

  void ClientBase::handleDecompressedData( const std::string& data )
  {
    parse( data );
  }

  void ClientBase::handleEncryptedData( const TLSBase* /*base*/, const std::string& data )
  {
    if( m_connection )
      m_connection->send( data );
    else
      m_logInstance.err( LogAreaClassClientbase, "Encryption finished, but chain broken" );
  }

  void ClientBase::handleDecryptedData( const TLSBase* /*base*/, const std::string& data )
  {
    if( m_compression && m_compressionActive )
      m_compression->decompress( data );
    else
      parse( data );
  }

  void ClientBase::handleHandshakeResult( const TLSBase* /*base*/, bool success, CertInfo &certinfo )
  {
    if( success )
    {
      if( !notifyOnTLSConnect( certinfo ) )
      {
        logInstance().err( LogAreaClassClientbase, "Server's certificate rejected!" );
        disconnect( ConnTlsFailed );
      }
      else
      {
        logInstance().dbg( LogAreaClassClientbase, "connection encryption active" );
        header();
      }
    }
    else
    {
      logInstance().err( LogAreaClassClientbase, "TLS handshake failed!" );
      disconnect( ConnTlsFailed );
    }
  }

  void ClientBase::handleReceivedData( const ConnectionBase* /*connection*/, const std::string& data )
  {
    if( m_encryption && m_encryptionActive )
      m_encryption->decrypt( data );
    else if( m_compression && m_compressionActive )
      m_compression->decompress( data );
    else
      parse( data );
  }

  void ClientBase::handleConnect( const ConnectionBase* /*connection*/ )
  {
    header();
  }

  void ClientBase::handleDisconnect( const ConnectionBase* /*connection*/, ConnectionError reason )
  {
    if( m_connection )
      m_connection->cleanup();

    if( m_encryption )
      m_encryption->cleanup();

    if( m_compression )
      m_compression->cleanup();

    m_encryptionActive = false;
    m_compressionActive = false;

    notifyOnDisconnect( reason );
  }

  void ClientBase::disconnect( ConnectionError reason )
  {
    if( !m_connection || m_connection->state() < StateConnecting )
      return;

    if( reason != ConnTlsFailed )
      send( "</stream:stream>" );

    m_connection->disconnect();
    m_connection->cleanup();

    if( m_encryption )
      m_encryption->cleanup();

    if( m_compression )
      m_compression->cleanup();

    m_encryptionActive = false;
    m_compressionActive = false;

    notifyOnDisconnect( reason );
  }

  void ClientBase::parse( const std::string& data )
  {
    std::string copy = data;
    int i = 0;
    if( ( i = m_parser.feed( copy ) ) >= 0 )
    {
      std::string error = "parse error (at pos ";
      error += util::int2string( i );
      error += "): ";
      m_logInstance.err( LogAreaClassClientbase, error + copy );
      Tag* e = new Tag( "stream:error" );
      new Tag( e, "restricted-xml", "xmlns", XMLNS_XMPP_STREAM );
      send( e );
      disconnect( ConnParseError );
    }
  }

  void ClientBase::header()
  {
    std::string head = "<?xml version='1.0' ?>";
    head += "<stream:stream to='" + m_jid.server() + "' xmlns='" + m_namespace + "' ";
    head += "xmlns:stream='http://etherx.jabber.org/streams'  xml:lang='" + m_xmllang + "' ";
    head += "version='" + XMPP_STREAM_VERSION_MAJOR + "." + XMPP_STREAM_VERSION_MINOR + "'>";
    send( head );
  }

  bool ClientBase::hasTls()
  {
#if defined( HAVE_GNUTLS ) || defined( HAVE_OPENSSL ) || defined( HAVE_WINTLS )
    return true;
#else
    return false;
#endif
  }

  void ClientBase::startTls()
  {
    send( new Tag( "starttls", XMLNS, XMLNS_STREAM_TLS ) );
  }

  void ClientBase::setServer( const std::string &server )
  {
    m_server = server;
    if( m_connection )
      m_connection->setServer( server );
  }

  void ClientBase::setClientCert( const std::string& clientKey, const std::string& clientCerts )
  {
    m_clientKey = clientKey;
    m_clientCerts = clientCerts;
  }

  void ClientBase::startSASL( SaslMechanism type )
  {
    m_selectedSaslMech = type;

    Tag* a = new Tag( "auth", XMLNS, XMLNS_STREAM_SASL );

    switch( type )
    {
      case SaslMechDigestMd5:
        a->addAttribute( "mechanism", "DIGEST-MD5" );
        break;
      case SaslMechPlain:
      {
        a->addAttribute( "mechanism", "PLAIN" );

        std::string tmp;
        if( m_authzid )
          tmp += m_authzid.bare();

        tmp += '\0';
        if( !m_authcid.empty() )
          tmp += m_authcid;
        else
          tmp += m_jid.username();
        tmp += '\0';
        tmp += m_password;
        a->setCData( Base64::encode64( tmp ) );
        break;
      }
      case SaslMechAnonymous:
        a->addAttribute( "mechanism", "ANONYMOUS" );
        break;
      case SaslMechExternal:
        a->addAttribute( "mechanism", "EXTERNAL" );
        a->setCData( Base64::encode64( m_authzid ? m_authzid.bare() : m_jid.bare() ) );
       break;
      case SaslMechGssapi:
      {
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
        a->addAttribute( "mechanism", "GSSAPI" );
// The client calls GSS_Init_sec_context, passing in 0 for
// input_context_handle (initially) and a targ_name equal to output_name
// from GSS_Import_Name called with input_name_type of
// GSS_C_NT_HOSTBASED_SERVICE and input_name_string of
// "service@hostname" where "service" is the service name specified in
// the protocol's profile, and "hostname" is the fully qualified host
// name of the server.  The client then responds with the resulting
// output_token.
        std::string token;
        a->setCData( Base64::encode64( token ) );
//         etc... see gssapi-sasl-draft.txt
#else
        logInstance().err( LogAreaClassClientbase,
                    "SASL GSSAPI is not supported on this platform. You should never see this." );
#endif
        break;
      }
      case SaslMechNTLM:
      {
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
        a->addAttribute( "mechanism", "NTLM" );
        SEC_WINNT_AUTH_IDENTITY identity, *ident = 0;
        memset( &identity, 0, sizeof( identity ) );
        if( m_jid.username().length() > 0 )
        {
          identity.User = (unsigned char*)m_jid.username().c_str();
          identity.UserLength = (unsigned long)m_jid.username().length();
          identity.Domain = (unsigned char*)m_ntlmDomain.c_str();
          identity.DomainLength = (unsigned long)m_ntlmDomain.length();
          identity.Password = (unsigned char*)m_password.c_str();
          identity.PasswordLength = (unsigned long)m_password.length();
          identity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
          ident = &identity;
        }
        AcquireCredentialsHandle( 0, _T( "NTLM" ), SECPKG_CRED_OUTBOUND, 0, ident, 0, 0, &m_credHandle, 0 );
#else
        logInstance().err( LogAreaClassClientbase,
                    "SASL NTLM is not supported on this platform. You should never see this." );
#endif
        break;
      }
      default:
        break;
    }

    send( a );
  }

  void ClientBase::processSASLChallenge( const std::string& challenge )
  {
    Tag* t = new Tag( "response", XMLNS, XMLNS_STREAM_SASL );

    const std::string& decoded = Base64::decode64( challenge );

    switch( m_selectedSaslMech )
    {
      case SaslMechDigestMd5:
      {
        if( !decoded.compare( 0, 7, "rspauth" ) )
          break;

        std::string realm;
        std::string::size_type end = 0;
        std::string::size_type pos = decoded.find( "realm=" );
        if( pos != std::string::npos )
        {
          end = decoded.find( '"', pos + 7 );
          realm = decoded.substr( pos + 7, end - ( pos + 7 ) );
        }
        else
          realm = m_jid.server();

        pos = decoded.find( "nonce=" );
        if( pos == std::string::npos )
          return;

        end = decoded.find( '"', pos + 7 );
        while( decoded[end-1] == '\\' )
          end = decoded.find( '"', end + 1 );
        std::string nonce = decoded.substr( pos + 7, end - ( pos + 7 ) );

        std::string cnonce;
        char cn[4*8+1];
        for( int i = 0; i < 4; ++i )
          sprintf( cn + i*8, "%08x", rand() );
        cnonce.assign( cn, 4*8 );

        MD5 md5;
        md5.feed( m_jid.username() );
        md5.feed( ":" );
        md5.feed( realm );
        md5.feed( ":" );
        md5.feed( m_password );
        md5.finalize();
        const std::string& a1_h = md5.binary();
        md5.reset();
        md5.feed( a1_h );
        md5.feed( ":" );
        md5.feed( nonce );
        md5.feed( ":" );
        md5.feed( cnonce );
        md5.finalize();
        const std::string& a1  = md5.hex();
        md5.reset();
        md5.feed( "AUTHENTICATE:xmpp/" );
        md5.feed( m_jid.server() );
        md5.finalize();
        const std::string& a2 = md5.hex();
        md5.reset();
        md5.feed( a1 );
        md5.feed( ":" );
        md5.feed( nonce );
        md5.feed( ":00000001:" );
        md5.feed( cnonce );
        md5.feed( ":auth:" );
        md5.feed( a2 );
        md5.finalize();

        std::string response = "username=\"";
        response += m_jid.username();
        response += "\",realm=\"";
        response += realm;
        response += "\",nonce=\"";
        response += nonce;
        response += "\",cnonce=\"";
        response += cnonce;
        response += "\",nc=00000001,qop=auth,digest-uri=\"xmpp/";
        response += m_jid.server();
        response += "\",response=";
        response += md5.hex();
        response += ",charset=utf-8";

        if( m_authzid )
          response += ",authzid=" + m_authzid.bare();

        t->setCData( Base64::encode64( response ) );

        break;
      }
      case SaslMechGssapi:
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
        // see gssapi-sasl-draft.txt
#else
        m_logInstance.err( LogAreaClassClientbase,
                           "Huh, received GSSAPI challenge?! This should have never happened!" );
#endif
        break;
      case SaslMechNTLM:
      {
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
        bool type1 = ( decoded.length() < 7 ) ? true : false;

        SecBuffer bufferIn = { type1 ? 0 : (unsigned long)decoded.length(),
                               SECBUFFER_TOKEN,
                               (void*)decoded.c_str() };
        SecBufferDesc secIn = { 0, 1, &bufferIn };

        char buffer[4096];

        SecBuffer bufferOut = { sizeof( buffer ), SECBUFFER_TOKEN, buffer };
        SecBufferDesc secOut = { 0, 1, &bufferOut };

        TimeStamp timestamp;
        unsigned long contextAttr;

        SECURITY_STATUS status = InitializeSecurityContext( &m_credHandle, type1 ? 0 : &m_ctxtHandle,
                                                            0, ISC_REQ_MUTUAL_AUTH, 0, 0, &secIn, 0,
                                                            &m_ctxtHandle, &secOut, &contextAttr,
                                                            &timestamp );
        std::string response;
        if( SUCCEEDED( status ) )
        {
          response = std::string( (const char *)bufferOut.pvBuffer, bufferOut.cbBuffer );
        }
        else
        {
          logInstance().err( LogAreaClassClientbase,
                             "InitializeSecurityContext() failed, return value "
                               + util::int2string( status ) );
        }

        t->setCData( Base64::encode64( response ) );
#else
        m_logInstance.err( LogAreaClassClientbase,
                           "Huh, received NTLM challenge?! This should have never happened!" );
#endif
        break;
      }

      default:
        // should never happen.
        break;
    }

    send( t );
  }

  void ClientBase::processSASLError( Tag* tag )
  {
    if( tag->hasChild( "aborted" ) )
      m_authError = SaslAborted;
    else if( tag->hasChild( "incorrect-encoding" ) )
      m_authError = SaslIncorrectEncoding;
    else if( tag->hasChild( "invalid-authzid" ) )
      m_authError = SaslInvalidAuthzid;
    else if( tag->hasChild( "invalid-mechanism" ) )
      m_authError = SaslInvalidMechanism;
    else if( tag->hasChild( "malformed-request" ) )
      m_authError = SaslMalformedRequest;
    else if( tag->hasChild( "mechanism-too-weak" ) )
      m_authError = SaslMechanismTooWeak;
    else if( tag->hasChild( "not-authorized" ) )
      m_authError = SaslNotAuthorized;
    else if( tag->hasChild( "temporary-auth-failure" ) )
      m_authError = SaslTemporaryAuthFailure;

#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
    if( m_selectedSaslMech == SaslMechNTLM )
    {
      FreeCredentialsHandle( &m_credHandle );
      DeleteSecurityContext( &m_ctxtHandle );
    }
#endif
  }

  void ClientBase::processSASLSuccess()
  {
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
    if( m_selectedSaslMech == SaslMechNTLM )
    {
      FreeCredentialsHandle( &m_credHandle );
      DeleteSecurityContext( &m_ctxtHandle );
    }
#endif
  }

  void ClientBase::send( IQ& iq, IqHandler* ih, int context, bool del )
  {
    if( ih && ( iq.subtype() == IQ::Set || iq.subtype() == IQ::Get ) )
    {
      if( iq.id().empty() )
        iq.setID( getID() );

      TrackStruct track;
      track.ih = ih;
      track.context = context;
      track.del = del;
      m_iqHandlerMapMutex.lock();
      m_iqIDHandlers[iq.id()] = track;
      m_iqHandlerMapMutex.unlock();
    }

    send( iq );
  }

  void ClientBase::send( const IQ& iq )
  {
    ++m_stats.iqStanzasSent;
    Tag* tag = iq.tag();
    addFrom( tag );
    addNamespace( tag );
    send( tag );
  }

  void ClientBase::send( const Message& msg )
  {
    ++m_stats.messageStanzasSent;
    Tag* tag = msg.tag();
    addFrom( tag );
    addNamespace( tag );
    send( tag );
  }

  void ClientBase::send( const Subscription& sub )
  {
    ++m_stats.s10nStanzasSent;
    Tag* tag = sub.tag();
    addFrom( tag );
    addNamespace( tag );
    send( tag );
  }

  void ClientBase::send( Presence& pres )
  {
    ++m_stats.presenceStanzasSent;
    Tag* tag = pres.tag();
    StanzaExtensionList::const_iterator it = m_presenceExtensions.begin();
    for( ; it != m_presenceExtensions.end(); ++it )
      tag->addChild( (*it)->tag() );
    addFrom( tag );
    addNamespace( tag );
    send( tag );
  }

  void ClientBase::send( Tag* tag )
  {
    if( !tag )
      return;

    send( tag->xml() );

    ++m_stats.totalStanzasSent;

    if( m_statisticsHandler )
      m_statisticsHandler->handleStatistics( getStatistics() );

    delete tag;
  }

  void ClientBase::send( const std::string& xml )
  {
    if( m_connection && m_connection->state() == StateConnected )
    {
      if( m_compression && m_compressionActive )
        m_compression->compress( xml );
      else if( m_encryption && m_encryptionActive )
        m_encryption->encrypt( xml );
      else
        m_connection->send( xml );

      logInstance().dbg( LogAreaXmlOutgoing, xml );
    }
  }

  void ClientBase::addFrom( Tag* tag )
  {
    if( !m_authed /*for IQ Auth */ || !tag || tag->hasAttribute( "from" ) )
      return;

    if ( m_selectedResource.empty() )
      tag->addAttribute( "from", m_jid.bare() );
    else
      tag->addAttribute( "from", m_jid.bare() + '/' + m_selectedResource );
  }

  void ClientBase::addNamespace( Tag* tag )
  {
    if( !tag || !tag->xmlns().empty() )
      return;

    tag->setXmlns( m_namespace );
  }

  void ClientBase::registerStanzaExtension( StanzaExtension* ext )
  {
    if( !m_seFactory )
      m_seFactory = new StanzaExtensionFactory();

    m_seFactory->registerExtension( ext );
  }

  bool ClientBase::removeStanzaExtension( int ext )
  {
    if( !m_seFactory )
      return false;

    return m_seFactory->removeExtension( ext );
  }

  StatisticsStruct ClientBase::getStatistics()
  {
    if( m_connection )
      m_connection->getStatistics( m_stats.totalBytesReceived, m_stats.totalBytesSent );

    return m_stats;
  }

  ConnectionState ClientBase::state() const
  {
    return m_connection ? m_connection->state() : StateDisconnected;
  }

  void ClientBase::whitespacePing()
  {
    send( " " );
  }

  void ClientBase::xmppPing( const JID& to, EventHandler* eh )
  {
    const std::string& id = getID();
    IQ iq( IQ::Get, to, id );
    iq.addExtension( new Ping() );
    m_dispatcher.registerEventHandler( eh, id );
    send( iq, this, XMPPPing );
  }

  bool ClientBase::handleIq( const IQ& iq )
  {
    const Ping* p = iq.findExtension<Ping>( ExtPing );
    if( !p || iq.subtype() != IQ::Get )
      return false;

    m_dispatcher.dispatch( Event( Event::PingPing, iq ) );
    IQ re( IQ::Result, iq.from(), iq.id() );
    send( re );

    return true;
  }

  void ClientBase::handleIqID( const IQ& iq, int context )
  {
    if( context == XMPPPing )
      m_dispatcher.dispatch( Event( ( iq.subtype() == IQ::Result ) ? Event::PingPong
                                                                   : Event::PingError, iq ),
                             iq.id(), true );
    else
      handleIqIDForward( iq, context );
  }

  const std::string ClientBase::getID()
  {
    static unsigned int uniqueBaseID = (unsigned int)time( 0 );
    char r[21+1];
    sprintf( r, "uid:%08x:%08x", uniqueBaseID, rand() );
    std::string ret( r, 21 );
    return ret;
  }

  bool ClientBase::checkStreamVersion( const std::string& version )
  {
    if( version.empty() )
      return false;

    int major = 0;
    int minor = 0;
    int myMajor = atoi( XMPP_STREAM_VERSION_MAJOR.c_str() );

    size_t dot = version.find( '.' );
    if( !version.empty() && dot && dot != std::string::npos )
    {
      major = atoi( version.substr( 0, dot ).c_str() );
      minor = atoi( version.substr( dot ).c_str() );
    }

    return myMajor >= major;
  }

  void ClientBase::setConnectionImpl( ConnectionBase* cb )
  {
    if( m_connection )
    {
      delete m_connection;
    }
    m_connection = cb;
  }

  void ClientBase::setEncryptionImpl( TLSBase* tb )
  {
    if( m_encryption )
    {
      delete m_encryption;
    }
    m_encryption = tb;
  }

  void ClientBase::setCompressionImpl( CompressionBase* cb )
  {
    if( m_compression )
    {
      delete m_compression;
    }
    m_compression = cb;
  }

  void ClientBase::handleStreamError( Tag* tag )
  {
    StreamError err = StreamErrorUndefined;
    const TagList& c = tag->children();
    TagList::const_iterator it = c.begin();
    for( ; it != c.end(); ++it )
    {
      const std::string& name = (*it)->name();
      if( name == "bad-format" )
        err = StreamErrorBadFormat;
      else if( name == "bad-namespace-prefix" )
        err = StreamErrorBadNamespacePrefix;
      else if( name == "conflict" )
        err = StreamErrorConflict;
      else if( name == "connection-timeout" )
        err = StreamErrorConnectionTimeout;
      else if( name == "host-gone" )
        err = StreamErrorHostGone;
      else if( name == "host-unknown" )
        err = StreamErrorHostUnknown;
      else if( name == "improper-addressing" )
        err = StreamErrorImproperAddressing;
      else if( name == "internal-server-error" )
        err = StreamErrorInternalServerError;
      else if( name == "invalid-from" )
        err = StreamErrorInvalidFrom;
      else if( name == "invalid-id" )
        err = StreamErrorInvalidId;
      else if( name == "invalid-namespace" )
        err = StreamErrorInvalidNamespace;
      else if( name == "invalid-xml" )
        err = StreamErrorInvalidXml;
      else if( name == "not-authorized" )
        err = StreamErrorNotAuthorized;
      else if( name == "policy-violation" )
        err = StreamErrorPolicyViolation;
      else if( name == "remote-connection-failed" )
        err = StreamErrorRemoteConnectionFailed;
      else if( name == "resource-constraint" )
        err = StreamErrorResourceConstraint;
      else if( name == "restricted-xml" )
        err = StreamErrorRestrictedXml;
      else if( name == "see-other-host" )
      {
        err = StreamErrorSeeOtherHost;
        m_streamErrorCData = tag->findChild( "see-other-host" )->cdata();
      }
      else if( name == "system-shutdown" )
        err = StreamErrorSystemShutdown;
      else if( name == "undefined-condition" )
        err = StreamErrorUndefinedCondition;
      else if( name == "unsupported-encoding" )
        err = StreamErrorUnsupportedEncoding;
      else if( name == "unsupported-stanza-type" )
        err = StreamErrorUnsupportedStanzaType;
      else if( name == "unsupported-version" )
        err = StreamErrorUnsupportedVersion;
      else if( name == "xml-not-well-formed" )
        err = StreamErrorXmlNotWellFormed;
      else if( name == "text" )
      {
        const std::string& lang = (*it)->findAttribute( "xml:lang" );
        if( !lang.empty() )
          m_streamErrorText[lang] = (*it)->cdata();
        else
          m_streamErrorText["default"] = (*it)->cdata();
      }
      else
        m_streamErrorAppCondition = (*it);

      if( err != StreamErrorUndefined && (*it)->hasAttribute( XMLNS, XMLNS_XMPP_STREAM ) )
        m_streamError = err;
    }
  }

  const std::string& ClientBase::streamErrorText( const std::string& lang ) const
  {
    StringMap::const_iterator it = m_streamErrorText.find( lang );
    return ( it != m_streamErrorText.end() ) ? (*it).second : EmptyString;
  }

  void ClientBase::registerMessageSessionHandler( MessageSessionHandler* msh, int types )
  {
    if( types & Message::Chat || types == 0 )
      m_messageSessionHandlerChat = msh;

    if( types & Message::Normal || types == 0 )
      m_messageSessionHandlerNormal = msh;

    if( types & Message::Groupchat || types == 0 )
      m_messageSessionHandlerGroupchat = msh;

    if( types & Message::Headline || types == 0 )
      m_messageSessionHandlerHeadline = msh;
  }

  void ClientBase::registerPresenceHandler( PresenceHandler* ph )
  {
    if( ph )
      m_presenceHandlers.push_back( ph );
  }

  void ClientBase::removePresenceHandler( PresenceHandler* ph )
  {
    if( ph )
      m_presenceHandlers.remove( ph );
  }

  void ClientBase::registerPresenceHandler( const JID& jid, PresenceHandler* ph )
  {
    if( ph && jid )
    {
      JidPresHandlerStruct jph;
      jph.jid = new JID( jid.bare() );
      jph.ph = ph;
      m_presenceJidHandlers.push_back( jph );
    }
  }

  void ClientBase::removePresenceHandler( const JID& jid, PresenceHandler* ph )
  {
    PresenceJidHandlerList::iterator t;
    PresenceJidHandlerList::iterator it = m_presenceJidHandlers.begin();
    while( it != m_presenceJidHandlers.end() )
    {
      t = it;
      ++it;
      if( ( !ph || (*t).ph == ph ) && (*t).jid->bare() == jid.bare() )
      {
        delete (*t).jid;
        m_presenceJidHandlers.erase( t );
      }
    }
  }

  void ClientBase::removeIDHandler( IqHandler* ih )
  {
    IqTrackMap::iterator t;
    m_iqHandlerMapMutex.lock();
    IqTrackMap::iterator it = m_iqIDHandlers.begin();
    while( it != m_iqIDHandlers.end() )
    {
      t = it;
      ++it;
      if( ih == (*t).second.ih )
        m_iqIDHandlers.erase( t );
    }
    m_iqHandlerMapMutex.unlock();
  }

  void ClientBase::registerIqHandler( IqHandler* ih, int exttype )
  {
    if( !ih )
      return;

    typedef IqHandlerMap::const_iterator IQci;
    std::pair<IQci, IQci> g = m_iqExtHandlers.equal_range( exttype );
    for( IQci it = g.first; it != g.second; ++it )
      if( (*it).second == ih )
        return;

    m_iqExtHandlers.insert( std::make_pair( exttype, ih ) );
  }

  void ClientBase::removeIqHandler( IqHandler* ih, int exttype )
  {
    if( !ih )
      return;

    typedef IqHandlerMap::iterator IQi;
    std::pair<IQi, IQi> g = m_iqExtHandlers.equal_range( exttype );
    IQi it2;
    IQi it = g.first;
    while( it != g.second )
    {
      it2 = it++;
      if( (*it2).second == ih )
        m_iqExtHandlers.erase( it2 );
    }
  }

  void ClientBase::registerMessageSession( MessageSession* session )
  {
    if( session )
      m_messageSessions.push_back( session );
  }

  void ClientBase::disposeMessageSession( MessageSession* session )
  {
    if( !session )
      return;

    MessageSessionList::iterator it = std::find( m_messageSessions.begin(),
                                                 m_messageSessions.end(),
                                                 session );
    if( it != m_messageSessions.end() )
    {
      delete (*it);
      m_messageSessions.erase( it );
    }
  }

  void ClientBase::registerMessageHandler( MessageHandler* mh )
  {
    if( mh )
      m_messageHandlers.push_back( mh );
  }

  void ClientBase::removeMessageHandler( MessageHandler* mh )
  {
    if( mh )
      m_messageHandlers.remove( mh );
  }

  void ClientBase::registerSubscriptionHandler( SubscriptionHandler* sh )
  {
    if( sh )
      m_subscriptionHandlers.push_back( sh );
  }

  void ClientBase::removeSubscriptionHandler( SubscriptionHandler* sh )
  {
    if( sh )
      m_subscriptionHandlers.remove( sh );
  }

  void ClientBase::registerTagHandler( TagHandler* th, const std::string& tag, const std::string& xmlns )
  {
    if( th && !tag.empty() )
    {
      TagHandlerStruct ths;
      ths.tag = tag;
      ths.xmlns = xmlns;
      ths.th = th;
      m_tagHandlers.push_back( ths );
    }
  }

  void ClientBase::removeTagHandler( TagHandler* th, const std::string& tag, const std::string& xmlns )
  {
    if( th )
    {
      TagHandlerList::iterator it = m_tagHandlers.begin();
      for( ; it != m_tagHandlers.end(); ++it )
      {
        if( (*it).th == th && (*it).tag == tag && (*it).xmlns == xmlns )
          m_tagHandlers.erase( it );
      }
    }
  }

  void ClientBase::registerStatisticsHandler( StatisticsHandler* sh )
  {
    if( sh )
      m_statisticsHandler = sh;
  }

  void ClientBase::removeStatisticsHandler()
  {
    m_statisticsHandler = 0;
  }

  void ClientBase::registerMUCInvitationHandler( MUCInvitationHandler* mih )
  {
    if( mih )
    {
      m_mucInvitationHandler = mih;
      m_disco->addFeature( XMLNS_MUC );
    }
  }

  void ClientBase::removeMUCInvitationHandler()
  {
    m_mucInvitationHandler = 0;
    m_disco->removeFeature( XMLNS_MUC );
  }

  void ClientBase::registerConnectionListener( ConnectionListener* cl )
  {
    if( cl )
      m_connectionListeners.push_back( cl );
  }

  void ClientBase::removeConnectionListener( ConnectionListener* cl )
  {
    if( cl )
      m_connectionListeners.remove( cl );
  }

  void ClientBase::notifyOnConnect()
  {
    util::ForEach( m_connectionListeners, &ConnectionListener::onConnect );
  }

  void ClientBase::notifyOnDisconnect( ConnectionError e )
  {
    util::ForEach( m_connectionListeners, &ConnectionListener::onDisconnect, e );
    init();
  }

  bool ClientBase::notifyOnTLSConnect( const CertInfo& info )
  {
    ConnectionListenerList::const_iterator it = m_connectionListeners.begin();
    for( ; it != m_connectionListeners.end() && (*it)->onTLSConnect( info ); ++it )
      ;
    return m_stats.encryption = ( it == m_connectionListeners.end() );
  }

  void ClientBase::notifyOnResourceBindError( const Error* error )
  {
    util::ForEach( m_connectionListeners, &ConnectionListener::onResourceBindError, error );
  }

  void ClientBase::notifyOnResourceBind( const std::string& resource )
  {
    util::ForEach( m_connectionListeners, &ConnectionListener::onResourceBind, resource );
  }

  void ClientBase::notifyOnSessionCreateError( const Error* error )
  {
    util::ForEach( m_connectionListeners, &ConnectionListener::onSessionCreateError, error );
  }

  void ClientBase::notifyStreamEvent( StreamEvent event )
  {
    util::ForEach( m_connectionListeners, &ConnectionListener::onStreamEvent, event );
  }

  void ClientBase::notifyPresenceHandlers( Presence& pres )
  {
    bool match = false;
    PresenceJidHandlerList::const_iterator t;
    PresenceJidHandlerList::const_iterator itj = m_presenceJidHandlers.begin();
    while( itj != m_presenceJidHandlers.end() )
    {
      t = itj++;
      if( (*t).jid->bare() == pres.from().bare() && (*t).ph )
      {
        (*t).ph->handlePresence( pres );
        match = true;
      }
    }
    if( match )
      return;

    // FIXME remove this for() for 1.1:
    PresenceHandlerList::const_iterator it = m_presenceHandlers.begin();
    for( ; it != m_presenceHandlers.end(); ++it )
    {
      (*it)->handlePresence( pres );
    }
      // FIXME and reinstantiate this:
//     util::ForEach( m_presenceHandlers, &PresenceHandler::handlePresence, pres );
  }

  void ClientBase::notifySubscriptionHandlers( Subscription& s10n )
  {
    // FIXME remove this for() for 1.1:
    SubscriptionHandlerList::const_iterator it = m_subscriptionHandlers.begin();
    for( ; it != m_subscriptionHandlers.end(); ++it )
    {
      (*it)->handleSubscription( s10n );
    }
      // FIXME and reinstantiate this:
//     util::ForEach( m_subscriptionHandlers, &SubscriptionHandler::handleSubscription, s10n );
  }

  void ClientBase::notifyIqHandlers( IQ& iq )
  {
    m_iqHandlerMapMutex.lock();
    IqTrackMap::iterator it_id = m_iqIDHandlers.find( iq.id() );
    m_iqHandlerMapMutex.unlock();
    if( it_id != m_iqIDHandlers.end() && iq.subtype() & ( IQ::Result | IQ::Error ) )
    {
      (*it_id).second.ih->handleIqID( iq, (*it_id).second.context );
      if( (*it_id).second.del )
        delete (*it_id).second.ih;
      m_iqHandlerMapMutex.lock();
      m_iqIDHandlers.erase( it_id );
      m_iqHandlerMapMutex.unlock();
      return;
    }

    if( iq.extensions().empty() )
      return;

    bool res = false;

    // FIXME remove for 1.1
//     typedef IqHandlerMapXmlns::const_iterator IQciXmlns
//     Tag *tag = iq.tag()->xmlns();
//     std::pair<IQciXmlns, IQciXmlns> g = m_iqNSHandlers.equal_range( tag->xmlns() );
//     for( IQciXmlns it = g.first; it != g.second; ++it )
//     {
//       if( (*it).second->handleIq( iq ) )
//         res = true;
//     }
//     delete tag;

    typedef IqHandlerMap::const_iterator IQci;
    const StanzaExtensionList& sel = iq.extensions();
    StanzaExtensionList::const_iterator itse = sel.begin();
    for( ; itse != sel.end(); ++itse )
    {
      std::pair<IQci, IQci> g = m_iqExtHandlers.equal_range( (*itse)->extensionType() );
      for( IQci it = g.first; it != g.second; ++it )
      {
        if( (*it).second->handleIq( iq ) )
          res = true;
      }
    }

    if( !res && iq.subtype() & ( IQ::Get | IQ::Set ) )
    {
      IQ re( IQ::Error, iq.from(), iq.id() );
      re.addExtension( new Error( StanzaErrorTypeCancel, StanzaErrorServiceUnavailable ) );
      send( re );
    }
  }

  void ClientBase::notifyMessageHandlers( Message& msg )
  {
    if( m_mucInvitationHandler )
    {
      const MUCRoom::MUCUser* mu = msg.findExtension<MUCRoom::MUCUser>( ExtMUCUser );
      if( mu && mu->operation() != MUCRoom::OpInviteTo )
      {

        m_mucInvitationHandler->handleMUCInvitation( msg.from(),
            mu->jid() ? JID( *(mu->jid()) ) : JID(),
            mu->reason() ? *(mu->reason()) : EmptyString,
            msg.body(),
            mu->password() ? *(mu->password()) : EmptyString,
            mu->continued(),
            mu->thread() ? *(mu->thread()) : EmptyString );
        return;
      }
    }

    MessageSessionList::const_iterator it1 = m_messageSessions.begin();
    for( ; it1 != m_messageSessions.end(); ++it1 )
    {
      if( (*it1)->target().full() == msg.from().full() &&
            ( msg.thread().empty()
              || (*it1)->threadID() == msg.thread()
              || (*it1)->honorThreadID() ) &&
// FIXME don't use '== 0' here
            ( (*it1)->types() & msg.subtype() || (*it1)->types() == 0 ) )
      {
        (*it1)->handleMessage( msg );
        return;
      }
    }

    it1 = m_messageSessions.begin();
    for( ; it1 != m_messageSessions.end(); ++it1 )
    {
      if( (*it1)->target().bare() == msg.from().bare() &&
            ( msg.thread().empty()
              || (*it1)->threadID() == msg.thread()
              || (*it1)->honorThreadID() ) &&
// FIXME don't use '== 0' here
            ( (*it1)->types() & msg.subtype() || (*it1)->types() == 0 ) )
      {
        (*it1)->handleMessage( msg );
        return;
      }
    }

    MessageSessionHandler* msHandler = 0;

    switch( msg.subtype() )
    {
      case Message::Chat:
        msHandler = m_messageSessionHandlerChat;
        break;
      case Message::Normal:
        msHandler = m_messageSessionHandlerNormal;
        break;
      case Message::Groupchat:
        msHandler = m_messageSessionHandlerGroupchat;
        break;
      case Message::Headline:
        msHandler = m_messageSessionHandlerHeadline;
        break;
      default:
        break;
    }

    if( msHandler )
    {
      if( msg.subtype() == Message::Chat && msg.body().empty() )
        return; // don't want a new MS for empty messages
      MessageSession* session = new MessageSession( this, msg.from(), true, msg.subtype() );
      msHandler->handleMessageSession( session );
      session->handleMessage( msg );
    }
    else
    {
      // FIXME remove this for() for 1.1:
      MessageHandlerList::const_iterator it = m_messageHandlers.begin();
      for( ; it != m_messageHandlers.end(); ++it )
      {
        (*it)->handleMessage( msg );
      }
      // FIXME and reinstantiate this:
//       util::ForEach( m_messageHandlers, &MessageHandler::handleMessage, msg ); // FIXME remove for 1.1
    }
  }

  void ClientBase::notifyTagHandlers( Tag* tag )
  {
    TagHandlerList::const_iterator it = m_tagHandlers.begin();
    for( ; it != m_tagHandlers.end(); ++it )
    {
      if( (*it).tag == tag->name() && tag->hasAttribute( XMLNS, (*it).xmlns ) )
        (*it).th->handleTag( tag );
    }
  }

  void ClientBase::addPresenceExtension( StanzaExtension* se )
  {
    if( !se )
      return;

    removePresenceExtension( se->extensionType() );
    m_presenceExtensions.push_back( se );
  }

  bool ClientBase::removePresenceExtension( int type )
  {
    StanzaExtensionList::iterator it = m_presenceExtensions.begin();
    for( ; it != m_presenceExtensions.end(); ++it )
    {
      if( (*it)->extensionType() == type )
      {
        delete (*it);
        m_presenceExtensions.erase( it );
        return true;
      }
    }

    return false;
  }

  CompressionBase* ClientBase::getDefaultCompression()
  {
    if( !m_compress )
      return 0;

#ifdef HAVE_ZLIB
    CompressionBase* cmp = new CompressionZlib( this );
    if( cmp->init() )
      return cmp;

    delete cmp;
#endif
    return 0;
  }

  TLSBase* ClientBase::getDefaultEncryption()
  {
    if( m_tls == TLSDisabled || !hasTls() )
      return 0;

    TLSDefault* tls = new TLSDefault( this, m_server );
    if( tls->init( m_clientKey, m_clientCerts, m_cacerts ) )
      return tls;
    else
    {
      delete tls;
      return 0;
    }
  }

}
