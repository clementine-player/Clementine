/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "config.h"

#include "gloox.h"

#include "connectionsocks5proxy.h"
#include "dns.h"
#include "logsink.h"
#include "prep.h"
#include "base64.h"
#include "util.h"

#include <string>
#include <cstdlib>

#include <string.h>

#if ( !defined( _WIN32 ) && !defined( _WIN32_WCE ) ) || defined( __SYMBIAN32__ )
# include <netinet/in.h>
#endif

#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
# include <winsock.h>
#elif defined( _WIN32_WCE )
# include <winsock2.h>
#endif

namespace gloox
{

  ConnectionSOCKS5Proxy::ConnectionSOCKS5Proxy( ConnectionBase* connection,
                                                const LogSink& logInstance,
                                                const std::string& server,
                                                int port, bool ip )
    : ConnectionBase( 0 ), m_connection( connection ),
      m_logInstance( logInstance ), m_s5state( S5StateDisconnected ), m_ip( ip )
  {
// FIXME check return value?
    prep::idna( server, m_server );
    m_port = port;

    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionSOCKS5Proxy::ConnectionSOCKS5Proxy( ConnectionDataHandler* cdh,
                                                ConnectionBase* connection,
                                                const LogSink& logInstance,
                                                const std::string& server,
                                                int port, bool ip )
    : ConnectionBase( cdh ), m_connection( connection ),
      m_logInstance( logInstance ), m_s5state( S5StateDisconnected ), m_ip( ip )
  {
// FIXME check return value?
    prep::idna( server, m_server );
    m_port = port;

    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionSOCKS5Proxy::~ConnectionSOCKS5Proxy()
  {
    if( m_connection )
      delete m_connection;
  }

  ConnectionBase* ConnectionSOCKS5Proxy::newInstance() const
  {
    ConnectionBase* conn = m_connection ? m_connection->newInstance() : 0;
    return new ConnectionSOCKS5Proxy( m_handler, conn, m_logInstance, m_server, m_port, m_ip );
  }

  void ConnectionSOCKS5Proxy::setConnectionImpl( ConnectionBase* connection )
  {
    if( m_connection )
      delete m_connection;

    m_connection = connection;
  }

  ConnectionError ConnectionSOCKS5Proxy::connect()
  {
// FIXME CHECKME
    if( m_connection && m_connection->state() == StateConnected && m_handler )
    {
      m_state = StateConnected;
      m_s5state = S5StateConnected;
      return ConnNoError;
    }

    if( m_connection && m_handler )
    {
      m_state = StateConnecting;
      m_s5state = S5StateConnecting;
      return m_connection->connect();
    }

    return ConnNotConnected;
  }

  void ConnectionSOCKS5Proxy::disconnect()
  {
    if( m_connection )
      m_connection->disconnect();
    cleanup();
  }

  ConnectionError ConnectionSOCKS5Proxy::recv( int timeout )
  {
    if( m_connection )
      return m_connection->recv( timeout );
    else
      return ConnNotConnected;
  }

  ConnectionError ConnectionSOCKS5Proxy::receive()
  {
    if( m_connection )
      return m_connection->receive();
    else
      return ConnNotConnected;
  }

  bool ConnectionSOCKS5Proxy::send( const std::string& data )
  {
//     if( m_s5state != S5StateConnected )
//     {
//       printf( "p data sent: " );
//       const char* x = data.c_str();
//       for( unsigned int i = 0; i < data.length(); ++i )
//         printf( "%02X ", (const char)x[i] );
//       printf( "\n" );
//     }

    if( m_connection )
      return m_connection->send( data );

    return false;
  }

  void ConnectionSOCKS5Proxy::cleanup()
  {
    m_state = StateDisconnected;
    m_s5state = S5StateDisconnected;

    if( m_connection )
      m_connection->cleanup();
  }

  void ConnectionSOCKS5Proxy::getStatistics( long int &totalIn, long int &totalOut )
  {
    if( m_connection )
      m_connection->getStatistics( totalIn, totalOut );
    else
    {
      totalIn = 0;
      totalOut = 0;
    }
  }

  void ConnectionSOCKS5Proxy::handleReceivedData( const ConnectionBase* /*connection*/,
                                                  const std::string& data )
  {
//     if( m_s5state != S5StateConnected )
//     {
//       printf( "data recv: " );
//       const char* x = data.c_str();
//       for( unsigned int i = 0; i < data.length(); ++i )
//         printf( "%02X ", (const char)x[i] );
//       printf( "\n" );
//     }

    if( !m_connection || !m_handler )
      return;

    ConnectionError connError = ConnNoError;

    switch( m_s5state  )
    {
      case S5StateConnecting:
        if( data.length() != 2 || data[0] != 0x05 )
          connError = ConnIoError;

        if( data[1] == 0x00 ) // no auth
        {
          negotiate();
        }
        else if( data[1] == 0x02 && !m_proxyUser.empty() && !m_proxyPwd.empty() ) // user/password auth
        {
          m_logInstance.dbg( LogAreaClassConnectionSOCKS5Proxy,
                             "authenticating to socks5 proxy as user " + m_proxyUser );
          m_s5state = S5StateAuthenticating;
          char* d = new char[3 + m_proxyUser.length() + m_proxyPwd.length()];
          size_t pos = 0;
          d[pos++] = 0x01;
          d[pos++] = (char)m_proxyUser.length();
          strncpy( d + pos, m_proxyUser.c_str(), m_proxyUser.length() );
          pos += m_proxyUser.length();
          d[pos++] = (char)m_proxyPwd.length();
          strncpy( d + pos, m_proxyPwd.c_str(), m_proxyPwd.length() );
          pos += m_proxyPwd.length();

          if( !send( std::string( d, pos ) ) )
          {
            cleanup();
            m_handler->handleDisconnect( this, ConnIoError );
          }
          delete[] d;
        }
        else
        {
          if( data[1] == (char)(unsigned char)0xFF && !m_proxyUser.empty() && !m_proxyPwd.empty() )
            connError = ConnProxyNoSupportedAuth;
          else
            connError = ConnProxyAuthRequired;
        }
        break;
      case S5StateNegotiating:
        if( data.length() >= 6 && data[0] == 0x05 )
        {
          if( data[1] == 0x00 )
          {
            m_state = StateConnected;
            m_s5state = S5StateConnected;
            m_handler->handleConnect( this );
          }
          else // connection refused
            connError = ConnConnectionRefused;
        }
        else
          connError = ConnIoError;
        break;
      case S5StateAuthenticating:
        if( data.length() == 2 && data[0] == 0x01 && data[1] == 0x00 )
          negotiate();
        else
          connError = ConnProxyAuthFailed;
        break;
      case S5StateConnected:
        m_handler->handleReceivedData( this, data );
        break;
      default:
        break;
    }

    if( connError != ConnNoError )
    {
      m_connection->disconnect();
      m_handler->handleDisconnect( this, connError );
    }

  }

  void ConnectionSOCKS5Proxy::negotiate()
  {
    m_s5state = S5StateNegotiating;
    char* d = new char[m_ip ? 10 : 6 + m_server.length() + 1];
    size_t pos = 0;
    d[pos++] = 0x05; // SOCKS version 5
    d[pos++] = 0x01; // command CONNECT
    d[pos++] = 0x00; // reserved
    int port = m_port;
    std::string server = m_server;
    if( m_ip ) // IP address
    {
      d[pos++] = 0x01; // IPv4 address
      std::string s;
      const size_t j = server.length();
      size_t l = 0;
      for( size_t k = 0; k < j && l < 4; ++k )
      {
        if( server[k] != '.' )
          s += server[k];

        if( server[k] == '.' || k == j-1 )
        {
          d[pos++] = static_cast<char>( atoi( s.c_str() ) & 0xFF );
          s = EmptyString;
          ++l;
        }
      }
    }
    else // hostname
    {
      if( port == -1 )
      {
        const DNS::HostMap& servers = DNS::resolve( m_server, m_logInstance );
        if( servers.size() )
        {
          const std::pair< std::string, int >& host = *servers.begin();
          server = host.first;
          port = host.second;
        }
      }
      d[pos++] = 0x03; // hostname
      d[pos++] = (char)m_server.length();
      strncpy( d + pos, m_server.c_str(), m_server.length() );
      pos += m_server.length();
    }
    int nport = htons( port );
    d[pos++] = static_cast<char>( nport );
    d[pos++] = static_cast<char>( nport >> 8 );

    std::string message = "Requesting socks5 proxy connection to " + server + ":"
        + util::int2string( port );
    m_logInstance.dbg( LogAreaClassConnectionSOCKS5Proxy, message );

    if( !send( std::string( d, pos ) ) )
    {
      cleanup();
      m_handler->handleDisconnect( this, ConnIoError );
    }
    delete[] d;
  }

  void ConnectionSOCKS5Proxy::handleConnect( const ConnectionBase* /*connection*/ )
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
      m_logInstance.dbg( LogAreaClassConnectionSOCKS5Proxy,
                         "Attempting to negotiate socks5 proxy connection" );

      const bool auth = !m_proxyUser.empty() && !m_proxyPwd.empty();
      const char d[4] = {
        0x05,                             // SOCKS version 5
        static_cast<char>( auth ? 0x02    // two methods
                                : 0x01 ), // one method
        0x00,                             // method: no auth
        0x02                              // method: username/password auth
      };

      if( !send( std::string( d, auth ? 4 : 3 ) ) )
      {
        cleanup();
        if( m_handler )
          m_handler->handleDisconnect( this, ConnIoError );
      }
    }
  }

  void ConnectionSOCKS5Proxy::handleDisconnect( const ConnectionBase* /*connection*/,
                                                ConnectionError reason )
  {
    cleanup();
    m_logInstance.dbg( LogAreaClassConnectionSOCKS5Proxy, "socks5 proxy connection closed" );

    if( m_handler )
      m_handler->handleDisconnect( this, reason );
  }

}
