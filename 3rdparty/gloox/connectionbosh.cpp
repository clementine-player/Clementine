/*
 * Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
 * This file is part of the gloox library. http://camaya.net/gloox
 *
 * This software is distributed under a license. The full license
 * agreement can be found in the file LICENSE in this distribution.
 * This software may not be copied, modified, sold or distributed
 * other than expressed in the named license agreement.
 *
 * This software is distributed without any warranty.
 */

#include "config.h"

#include "gloox.h"

#include "connectionbosh.h"
#include "logsink.h"
#include "prep.h"
#include "tag.h"
#include "util.h"

#include <string>
#include <cstdlib>
#include <cctype>
#include <algorithm>

namespace gloox
{

  ConnectionBOSH::ConnectionBOSH( ConnectionBase* connection, const LogSink& logInstance,
                                  const std::string& boshHost, const std::string& xmppServer,
                                  int xmppPort )
    : ConnectionBase( 0 ),
      m_logInstance( logInstance ), m_parser( this ), m_boshHost( boshHost ), m_path( "/http-bind/" ),
      m_rid( 0 ), m_initialStreamSent( false ), m_openRequests( 0 ),
      m_maxOpenRequests( 2 ), m_wait( 30 ), m_hold( 2 ), m_streamRestart( false ),
      m_lastRequestTime( std::time( 0 ) ), m_minTimePerRequest( 0 ), m_bufferContentLength( 0 ),
      m_connMode( ModePipelining )
  {
    initInstance( connection, xmppServer, xmppPort );
  }

  ConnectionBOSH::ConnectionBOSH( ConnectionDataHandler* cdh, ConnectionBase* connection,
                                  const LogSink& logInstance, const std::string& boshHost,
                                  const std::string& xmppServer, int xmppPort )
    : ConnectionBase( cdh ),
      m_logInstance( logInstance ), m_parser( this ), m_boshHost( boshHost ), m_path( "/http-bind/" ),
      m_rid( 0 ),  m_initialStreamSent( false ), m_openRequests( 0 ),
      m_maxOpenRequests( 2 ), m_wait( 30 ), m_hold( 2 ), m_streamRestart( false ),
      m_lastRequestTime( std::time( 0 ) ), m_minTimePerRequest( 0 ), m_bufferContentLength( 0 ),
      m_connMode( ModePipelining )
  {
    initInstance( connection, xmppServer, xmppPort );
  }

  void ConnectionBOSH::initInstance( ConnectionBase* connection, const std::string& xmppServer,
                                     const int xmppPort )
  {
// FIXME: check return value
    prep::idna( xmppServer, m_server );
    m_port = xmppPort;
    if( m_port != -1 )
    {
      m_boshedHost = m_boshHost + ":" + util::int2string( m_port );
    }

    // drop this connection into our pool of available connections
    if( connection )
    {
      connection->registerConnectionDataHandler( this );
      m_connectionPool.push_back( connection );
    }
  }

  ConnectionBOSH::~ConnectionBOSH()
  {
    util::clearList( m_activeConnections );
    util::clearList( m_connectionPool );
  }

  ConnectionBase* ConnectionBOSH::newInstance() const
  {
    ConnectionBase* pBaseConn = 0;

    if( !m_connectionPool.empty() )
    {
      pBaseConn = m_connectionPool.front()->newInstance();
    }
    else if( !m_activeConnections.empty() )
    {
      pBaseConn = m_activeConnections.front()->newInstance();
    }
    else
    {
      return 0;
    }

    return new ConnectionBOSH( m_handler, pBaseConn, m_logInstance,
                               m_boshHost, m_server, m_port );
  }

  ConnectionError ConnectionBOSH::connect()
  {
    if( m_state >= StateConnecting )
      return ConnNoError;

    if( !m_handler )
      return ConnNotConnected;

    m_state = StateConnecting;
    m_logInstance.dbg( LogAreaClassConnectionBOSH,
                       "bosh initiating connection to server: " +
                       ( ( m_connMode == ModePipelining ) ? std::string( "Pipelining" )
                         : ( ( m_connMode == ModeLegacyHTTP ) ? std::string( "LegacyHTTP" )
                                                              : std::string( "PersistentHTTP" ) ) ) );
    getConnection();
    return ConnNoError; // FIXME?
  }

  void ConnectionBOSH::disconnect()
  {
    if( ( m_connMode == ModePipelining && m_activeConnections.empty() )
        || ( m_connectionPool.empty() && m_activeConnections.empty() ) )
      return;

    if( m_state != StateDisconnected )
    {
      ++m_rid;

      std::string requestBody = "<body rid='" + util::int2string( m_rid ) + "' ";
      requestBody += "sid='" + m_sid + "' ";
      requestBody += "type='terminal' ";
      requestBody += "xml:lang='en' ";
      requestBody += "xmlns='" + XMLNS_HTTPBIND + "'";
      if( m_sendBuffer.empty() )  // Make sure that any data in the send buffer gets sent
        requestBody += "/>";
      else
      {
        requestBody += ">" + m_sendBuffer + "</body>";
        m_sendBuffer = EmptyString;
      }
      sendRequest( requestBody );

      m_logInstance.dbg( LogAreaClassConnectionBOSH, "bosh disconnection request sent" );
    }
    else
    {
      m_logInstance.err( LogAreaClassConnectionBOSH,
                         "disconnecting from server in a non-graceful fashion" );
    }

    util::ForEach( m_activeConnections, &ConnectionBase::disconnect );
    util::ForEach( m_connectionPool, &ConnectionBase::disconnect );

    m_state = StateDisconnected;
    if( m_handler )
      m_handler->handleDisconnect( this, ConnUserDisconnected );
  }

  ConnectionError ConnectionBOSH::recv( int timeout )
  {
    if( m_state == StateDisconnected )
      return ConnNotConnected;

    if( !m_connectionPool.empty() )
      m_connectionPool.front()->recv( 0 );
    if( !m_activeConnections.empty() )
      m_activeConnections.front()->recv( timeout );

    // If there are no open requests then the spec allows us to send an empty request...
    // (Some CMs do not obey this, it seems)
    if( ( m_openRequests == 0 || m_sendBuffer.size() > 0 ) && m_state == StateConnected )
    {
      m_logInstance.dbg( LogAreaClassConnectionBOSH,
                         "Sending empty request (or there is data in the send buffer)" );
      sendXML();
    }

    return ConnNoError; // FIXME?
  }

  bool ConnectionBOSH::send( const std::string& data )
  {

    if( m_state == StateDisconnected )
      return false;

    if( data.substr( 0, 2 ) == "<?" )
    {
//       if( m_initialStreamSent )
      {
        m_streamRestart = true;
        sendXML();
        return true;
      }
//       else
//       {
//         m_initialStreamSent = true;
//         m_logInstance.dbg( LogAreaClassConnectionBOSH, "initial <stream:stream> dropped" );
//         return true;
//       }
    }
    else if( data == "</stream:stream>" )
      return true;

    m_sendBuffer += data;
    sendXML();

    return true;
  }

  /* Sends XML. Wraps data in a <body/> tag, and then passes to sendRequest(). */
  bool ConnectionBOSH::sendXML()
  {
    if( m_state != StateConnected )
    {
      m_logInstance.warn( LogAreaClassConnectionBOSH,
                         "Data sent before connection established (will be buffered)" );
      return false;
    }

    if( m_sendBuffer.empty() )
    {
      time_t now = time( 0 );
      unsigned int delta = (int)(now - m_lastRequestTime);
      if( delta < m_minTimePerRequest && m_openRequests > 0 )
      {
        m_logInstance.dbg( LogAreaClassConnectionBOSH, "Too little time between requests: " + util::int2string( delta ) + " seconds" );
        return false;
      }
      m_logInstance.dbg( LogAreaClassConnectionBOSH, "Send buffer is empty, sending empty request" );
    }

    ++m_rid;

    std::string requestBody = "<body rid='" + util::int2string( m_rid ) + "' ";
    requestBody += "sid='" + m_sid + "' ";
    requestBody += "xmlns='" + XMLNS_HTTPBIND + "'";

    if( m_streamRestart )
    {
      requestBody += " xmpp:restart='true' to='" + m_server + "' xml:lang='en' xmlns:xmpp='"
          + XMLNS_XMPP_BOSH + "' />";
      m_logInstance.dbg( LogAreaClassConnectionBOSH, "Restarting stream" );
    }
    else
    {
      requestBody += ">" + m_sendBuffer + "</body>";
    }
    // Send a request. Force if we are not sending an empty request, or if there are no connections open
    if( sendRequest( requestBody ) )
    {
      m_logInstance.dbg( LogAreaClassConnectionBOSH, "Successfully sent m_sendBuffer" );
      m_sendBuffer = EmptyString;
      m_streamRestart = false;
    }
    else
    {
      --m_rid; // I think... (may need to rethink when acks are implemented)
      m_logInstance.warn( LogAreaClassConnectionBOSH,
                         "Unable to send. Connection not complete, or too many open requests,"
                         " so added to buffer.\n" );
    }

    return true;
  }

  /* Chooses the appropriate connection, or opens a new one if necessary. Wraps xml in HTTP and sends. */
  bool ConnectionBOSH::sendRequest( const std::string& xml )
  {
    ConnectionBase* conn = getConnection();
    if( !conn )
      return false;

    std::string request = "POST " + m_path;
    if( m_connMode == ModeLegacyHTTP )
    {
      request += " HTTP/1.0\r\n";
      request += "Connection: close\r\n";
    }
    else
      request += " HTTP/1.1\r\n";

    request += "Host: " + m_boshedHost + "\r\n";
    request += "Content-Type: text/xml; charset=utf-8\r\n";
    request += "Content-Length: " + util::int2string( xml.length() ) + "\r\n";
    request += "User-Agent: gloox/" + GLOOX_VERSION + "\r\n\r\n";
    request += xml;


    if( conn->send( request ) )
    {
      m_lastRequestTime = time( 0 );
      ++m_openRequests;
      return true;
    }
//     else // FIXME What to do in this case?
//       printf( "Error while trying to send on socket (state: %d)\n", conn->state() );

    return false;
  }

  bool ci_equal( char ch1, char ch2 )
  {
    return std::toupper( (unsigned char)ch1 ) == std::toupper( (unsigned char)ch2 );
  }

  std::string::size_type ci_find( const std::string& str1, const std::string& str2 )
  {
    std::string::const_iterator pos = std::search( str1.begin(), str1.end(),
                                                   str2.begin(), str2.end(), ci_equal );
    if( pos == str1.end() )
      return std::string::npos;
    else
      return std::distance( str1.begin(), pos );
  }

  const std::string ConnectionBOSH::getHTTPField( const std::string& field )
  {
    std::string::size_type fp = ci_find( m_bufferHeader, "\r\n" + field + ": " );

    if( fp == std::string::npos )
      return EmptyString;

    fp += field.length() + 4;

    const std::string::size_type fp2 = m_bufferHeader.find( "\r\n", fp );
    if( fp2 == std::string::npos )
      return EmptyString;

    return m_bufferHeader.substr( fp, fp2 - fp );
  }

  ConnectionError ConnectionBOSH::receive()
  {
    ConnectionError err = ConnNoError;
    while( m_state != StateDisconnected && ( err = recv( 10 ) ) == ConnNoError )
      ;
    return err == ConnNoError ? ConnNotConnected : err;
  }

  void ConnectionBOSH::cleanup()
  {
    m_state = StateDisconnected;

    util::ForEach( m_activeConnections, &ConnectionBase::cleanup );
    util::ForEach( m_connectionPool, &ConnectionBase::cleanup );
  }

  void ConnectionBOSH::getStatistics( long int& totalIn, long int& totalOut )
  {
    util::ForEach( m_activeConnections, &ConnectionBase::getStatistics, totalIn, totalOut );
    util::ForEach( m_connectionPool, &ConnectionBase::getStatistics, totalIn, totalOut );
  }

  void ConnectionBOSH::handleReceivedData( const ConnectionBase* /*connection*/,
                                           const std::string& data )
  {
    m_buffer += data;
    std::string::size_type headerLength = 0;
    while( ( headerLength = m_buffer.find( "\r\n\r\n" ) ) != std::string::npos )
    {
      m_bufferHeader = m_buffer.substr( 0, headerLength+2 );

      const std::string& statusCode = m_bufferHeader.substr( 9, 3 );
      if( statusCode != "200" )
      {
        m_logInstance.warn( LogAreaClassConnectionBOSH,
                            "Received error via legacy HTTP status code: " + statusCode
                                + ". Disconnecting." );
        m_state = StateDisconnected; // As per XEP, consider connection broken
        disconnect();
      }

      m_bufferContentLength = atol( getHTTPField( "Content-Length" ).c_str() );
      if( !m_bufferContentLength )
        return;

      if( m_connMode != ModeLegacyHTTP && ( getHTTPField( "Connection" ) == "close"
                                            || m_bufferHeader.substr( 0, 8 ) == "HTTP/1.0" ) )
      {
        m_logInstance.dbg( LogAreaClassConnectionBOSH,
                            "Server indicated lack of support for HTTP/1.1 - falling back to HTTP/1.0" );
        m_connMode = ModeLegacyHTTP;
      }

      if( m_buffer.length() >= ( headerLength + 4 + m_bufferContentLength ) )
      {
        putConnection();
        --m_openRequests;
        std::string xml = m_buffer.substr( headerLength + 4, m_bufferContentLength );
        m_parser.feed( xml );
        m_buffer.erase( 0, headerLength + 4 + m_bufferContentLength );
        m_bufferContentLength = 0;
        m_bufferHeader = EmptyString;
      }
      else
      {
        m_logInstance.warn( LogAreaClassConnectionBOSH, "buffer length mismatch" );
        break;
      }
    }
  }

  void ConnectionBOSH::handleConnect( const ConnectionBase* /*connection*/ )
  {
    if( m_state == StateConnecting )
    {
      m_rid = rand() % 100000 + 1728679472;

      Tag requestBody( "body" );
      requestBody.setXmlns( XMLNS_HTTPBIND );
      requestBody.setXmlns( XMLNS_XMPP_BOSH, "xmpp" );

      requestBody.addAttribute( "content", "text/xml; charset=utf-8" );
      requestBody.addAttribute( "hold", (long)m_hold );
      requestBody.addAttribute( "rid", (long)m_rid );
      requestBody.addAttribute( "ver", "1.6" );
      requestBody.addAttribute( "wait", (long)m_wait );
      requestBody.addAttribute( "ack", 0 );
      requestBody.addAttribute( "secure", "false" );
      requestBody.addAttribute( "route", "xmpp:" + m_server + ":5222" );
      requestBody.addAttribute( "xml:lang", "en" );
      requestBody.addAttribute( "xmpp:version", "1.0" );
      requestBody.addAttribute( "to", m_server );

      m_logInstance.dbg( LogAreaClassConnectionBOSH, "sending bosh connection request" );
      sendRequest( requestBody.xml() );
    }
  }

  void ConnectionBOSH::handleDisconnect( const ConnectionBase* /*connection*/,
                                         ConnectionError reason )
  {
    if( m_handler && m_state == StateConnecting )
    {
      m_state = StateDisconnected;
      m_handler->handleDisconnect( this, reason );
      return;
    }

    switch( m_connMode ) // FIXME avoid that if we're disconnecting on purpose
    {
      case ModePipelining:
        m_connMode = ModeLegacyHTTP; // Server seems not to support pipelining
        m_logInstance.dbg( LogAreaClassConnectionBOSH,
                           "connection closed - falling back to HTTP/1.0 connection method" );
        break;
      case ModeLegacyHTTP:
      case ModePersistentHTTP:
        // FIXME do we need to do anything here?
//         printf( "A TCP connection %p was disconnected (reason: %d).\n", connection, reason );
        break;
    }
  }

  void ConnectionBOSH::handleTag( Tag* tag )
  {
    if( !m_handler || tag->name() != "body" )
      return;

    if( m_streamRestart )
    {
      m_streamRestart = false;
      m_logInstance.dbg( LogAreaClassConnectionBOSH, "sending spoofed <stream:stream>" );
      m_handler->handleReceivedData( this, "<?xml version='1.0' ?>"
          "<stream:stream xmlns:stream='http://etherx.jabber.org/streams'"
          " xmlns='" + XMLNS_CLIENT + "' version='" + XMPP_STREAM_VERSION_MAJOR
          + "." + XMPP_STREAM_VERSION_MINOR + "' from='" + m_server + "' id ='"
          + m_sid + "' xml:lang='en'>" );
    }

    if( tag->hasAttribute( "sid" ) )
    {
      m_state = StateConnected;
      m_sid = tag->findAttribute( "sid" );

      if( tag->hasAttribute( "requests" ) )
      {
        const int serverRequests = atoi( tag->findAttribute( "requests" ).c_str() );
        if( serverRequests < m_maxOpenRequests )
        {
          m_maxOpenRequests = serverRequests;
          m_logInstance.dbg( LogAreaClassConnectionBOSH,
                              "bosh parameter 'requests' now set to " + tag->findAttribute( "requests" ) );
        }
      }
      if( tag->hasAttribute( "hold" ) )
      {
        const int maxHold = atoi( tag->findAttribute( "hold" ).c_str() );
        if( maxHold < m_hold )
        {
          m_hold = maxHold;
          m_logInstance.dbg( LogAreaClassConnectionBOSH,
                              "bosh parameter 'hold' now set to " + tag->findAttribute( "hold" ) );
        }
      }
      if( tag->hasAttribute( "wait" ) )
      {
        const int maxWait = atoi( tag->findAttribute( "wait" ).c_str() );
        if( maxWait < m_wait )
        {
          m_wait = maxWait;
          m_logInstance.dbg( LogAreaClassConnectionBOSH,
                              "bosh parameter 'wait' now set to " + tag->findAttribute( "wait" )
                                  + " seconds" );
        }
      }
      if( tag->hasAttribute( "polling" ) )
      {
        const int minTime = atoi( tag->findAttribute( "polling" ).c_str() );
        m_minTimePerRequest = minTime;
        m_logInstance.dbg( LogAreaClassConnectionBOSH,
                            "bosh parameter 'polling' now set to " + tag->findAttribute( "polling" )
                                + " seconds" );
      }

      if( m_state < StateConnected )
        m_handler->handleConnect( this );

      m_handler->handleReceivedData( this, "<?xml version='1.0' ?>" // FIXME move to send() so that
                                                                    // it is more clearly a response
                                                                    // to the initial stream opener?
                          "<stream:stream xmlns:stream='http://etherx.jabber.org/streams' "
                          "xmlns='" + XMLNS_CLIENT
                          + "' version='" + XMPP_STREAM_VERSION_MAJOR + "." + XMPP_STREAM_VERSION_MINOR
                          + "' from='" + m_server + "' id ='" + m_sid + "' xml:lang='en'>" );
    }

    if( tag->findAttribute( "type" ) == "terminate" )
    {
      m_logInstance.dbg( LogAreaClassConnectionBOSH,
                         "bosh connection closed by server: " + tag->findAttribute( "condition" ) );
      m_state = StateDisconnected;
      m_handler->handleDisconnect( this, ConnStreamClosed );
      return;
    }

    const TagList& stanzas = tag->children();
    TagList::const_iterator it = stanzas.begin();
    for( ; it != stanzas.end(); ++it )
      m_handler->handleReceivedData( this, (*it)->xml() );
  }

  ConnectionBase* ConnectionBOSH::getConnection()
  {
    if( m_openRequests > 0 && m_openRequests >= m_maxOpenRequests )
    {
      m_logInstance.warn( LogAreaClassConnectionBOSH,
                          "Too many requests already open. Cannot send." );
      return 0;
    }

    ConnectionBase* conn = 0;
    switch( m_connMode )
    {
      case ModePipelining:
        if( !m_activeConnections.empty() )
        {
          m_logInstance.dbg( LogAreaClassConnectionBOSH, "Using default connection for Pipelining." );
          return m_activeConnections.front();
        }
        else if( !m_connectionPool.empty() )
        {
          m_logInstance.warn( LogAreaClassConnectionBOSH,
                              "Pipelining selected, but no connection open. Opening one." );
          return activateConnection();
        }
        else
          m_logInstance.warn( LogAreaClassConnectionBOSH,
                              "No available connections to pipeline on." );
        break;
      case ModeLegacyHTTP:
      case ModePersistentHTTP:
      {
        if( !m_connectionPool.empty() )
        {
          m_logInstance.dbg( LogAreaClassConnectionBOSH, "LegacyHTTP/PersistentHTTP selected, "
                                                         "using connection from pool." );
          return activateConnection();
        }
        else if( !m_activeConnections.empty() )
        {
          m_logInstance.dbg( LogAreaClassConnectionBOSH, "No connections in pool, creating a new one." );
          conn = m_activeConnections.front()->newInstance();
          conn->registerConnectionDataHandler( this );
          m_connectionPool.push_back( conn );
          conn->connect();
        }
        else
          m_logInstance.warn( LogAreaClassConnectionBOSH,
                              "No available connections to send on." );
        break;
      }
    }
    return 0;
  }

  ConnectionBase* ConnectionBOSH::activateConnection()
  {
    ConnectionBase* conn = m_connectionPool.front();
    m_connectionPool.pop_front();
    if( conn->state() == StateConnected )
    {
      m_activeConnections.push_back( conn );
      return conn;
    }

    m_logInstance.dbg( LogAreaClassConnectionBOSH, "Connecting pooled connection." );
    m_connectionPool.push_back( conn );
    conn->connect();
    return 0;
  }

  void ConnectionBOSH::putConnection()
  {
    ConnectionBase* conn = m_activeConnections.front();

    switch( m_connMode )
    {
      case ModeLegacyHTTP:
        m_logInstance.dbg( LogAreaClassConnectionBOSH, "Disconnecting LegacyHTTP connection" );
        conn->disconnect();
        conn->cleanup(); // This is necessary
        m_activeConnections.pop_front();
        m_connectionPool.push_back( conn );
        break;
      case ModePersistentHTTP:
        m_logInstance.dbg( LogAreaClassConnectionBOSH, "Deactivating PersistentHTTP connection" );
        m_activeConnections.pop_front();
        m_connectionPool.push_back( conn );
        break;
      case ModePipelining:
        m_logInstance.dbg( LogAreaClassConnectionBOSH, "Keeping Pipelining connection" );
      default:
        break;
    }
  }

}
