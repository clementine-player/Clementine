/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "gloox.h"

#include "connectionhttpproxy.h"
#include "dns.h"
#include "logsink.h"
#include "prep.h"
#include "base64.h"
#include "util.h"

#include <string>

namespace gloox
{

  ConnectionHTTPProxy::ConnectionHTTPProxy( ConnectionBase* connection,
                                            const LogSink& logInstance,
                                            const std::string& server, int port )
    : ConnectionBase( 0 ), m_connection( connection ),
      m_logInstance( logInstance ), m_http11( false )
  {
// FIXME check return value?
    prep::idna( server, m_server );
    m_port = port;

    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionHTTPProxy::ConnectionHTTPProxy( ConnectionDataHandler* cdh,
                                            ConnectionBase* connection,
                                            const LogSink& logInstance,
                                            const std::string& server, int port )
    : ConnectionBase( cdh ), m_connection( connection ),
      m_logInstance( logInstance )
  {
// FIXME check return value?
    prep::idna( server, m_server );
    m_port = port;

    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionHTTPProxy::~ConnectionHTTPProxy()
  {
    delete m_connection;
  }

  ConnectionBase* ConnectionHTTPProxy::newInstance() const
  {
    ConnectionBase* conn = m_connection ? m_connection->newInstance() : 0;
    return new ConnectionHTTPProxy( m_handler, conn, m_logInstance, m_server, m_port );
  }

  void ConnectionHTTPProxy::setConnectionImpl( ConnectionBase* connection )
  {
    if( m_connection )
      delete m_connection;

    m_connection = connection;
  }

  ConnectionError ConnectionHTTPProxy::connect()
  {
    if( m_connection && m_handler )
    {
      m_state = StateConnecting;
      return m_connection->connect();
    }

    return ConnNotConnected;
  }

  void ConnectionHTTPProxy::disconnect()
  {
    m_state = StateDisconnected;
    if( m_connection )
      m_connection->disconnect();
  }

  ConnectionError ConnectionHTTPProxy::recv( int timeout )
  {
    return m_connection ? m_connection->recv( timeout ) : ConnNotConnected;
  }

  ConnectionError ConnectionHTTPProxy::receive()
  {
    return m_connection ? m_connection->receive() : ConnNotConnected;
  }

  bool ConnectionHTTPProxy::send( const std::string& data )
  {
    return m_connection && m_connection->send( data );
  }

  void ConnectionHTTPProxy::cleanup()
  {
    m_state = StateDisconnected;

    if( m_connection )
      m_connection->cleanup();
  }

  void ConnectionHTTPProxy::getStatistics( long int& totalIn, long int& totalOut )
  {
    if( m_connection )
      m_connection->getStatistics( totalIn, totalOut );
    else
      totalIn = totalOut = 0;
  }

  void ConnectionHTTPProxy::handleReceivedData( const ConnectionBase* /*connection*/,
                                                const std::string& data )
  {
    if( !m_handler )
      return;

    if( m_state == StateConnecting )
    {
      m_proxyHandshakeBuffer += data;
      if( ( !m_proxyHandshakeBuffer.compare( 0, 12, "HTTP/1.0 200" )
         || !m_proxyHandshakeBuffer.compare( 0, 12, "HTTP/1.1 200" ) )
         && !m_proxyHandshakeBuffer.compare( m_proxyHandshakeBuffer.length() - 4, 4, "\r\n\r\n" ) )
      {
        m_proxyHandshakeBuffer = EmptyString;
        m_state = StateConnected;
        m_logInstance.dbg( LogAreaClassConnectionHTTPProxy,
                           "http proxy connection established" );
        m_handler->handleConnect( this );
      }
      else if( !m_proxyHandshakeBuffer.compare( 9, 3, "407" ) )
      {
        m_handler->handleDisconnect( this, ConnProxyAuthRequired );
        m_connection->disconnect();
      }
      else if( !m_proxyHandshakeBuffer.compare( 9, 3, "403" )
            || !m_proxyHandshakeBuffer.compare( 9, 3, "404" ) )
      {
        m_handler->handleDisconnect( this, ConnProxyAuthFailed );
        m_connection->disconnect();
      }
    }
    else if( m_state == StateConnected )
      m_handler->handleReceivedData( this, data );
  }

  void ConnectionHTTPProxy::handleConnect( const ConnectionBase* /*connection*/ )
  {
    if( m_connection )
    {
      std::string server = m_server;
      int port = m_port;
      if( port == -1 )
      {
        const DNS::HostMap& servers = DNS::resolve( m_server, m_logInstance );
        if( !servers.empty() )
        {
          const std::pair< std::string, int >& host = *servers.begin();
          server = host.first;
          port = host.second;
        }
      }
      std::string message = "Requesting http proxy connection to " + server + ":"
          + util::int2string( port );
      m_logInstance.dbg( LogAreaClassConnectionHTTPProxy, message );

      std::string os = "CONNECT " + server + ":" + util::int2string( port ) + " HTTP/1."
          + util::int2string( m_http11 ? 1 : 0 ) + "\r\n"
          "Host: " + server + "\r\n"
          "Content-Length: 0\r\n"
          "Proxy-Connection: Keep-Alive\r\n"
          "Pragma: no-cache\r\n"
          "User-Agent: gloox/" + GLOOX_VERSION + "\r\n";

      if( !m_proxyUser.empty() && !m_proxyPwd.empty() )
      {
        os += "Proxy-Authorization: Basic " + Base64::encode64( m_proxyUser + ":" + m_proxyPwd )
            + "\r\n";
      }
      os += "\r\n";

      if( !m_connection->send( os ) )
      {
        m_state = StateDisconnected;
        if( m_handler )
          m_handler->handleDisconnect( this, ConnIoError );
      }
    }
  }

  void ConnectionHTTPProxy::handleDisconnect( const ConnectionBase* /*connection*/,
                                              ConnectionError reason )
  {
    m_state = StateDisconnected;
    m_logInstance.dbg( LogAreaClassConnectionHTTPProxy, "HTTP Proxy connection closed" );

    if( m_handler )
      m_handler->handleDisconnect( this, reason );
  }

}
