/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "socks5bytestreamserver.h"
#include "connectiontcpserver.h"
#include "mutexguard.h"
#include "util.h"

namespace gloox
{

  SOCKS5BytestreamServer::SOCKS5BytestreamServer( const LogSink& logInstance, int port,
                                                  const std::string& ip )
    : m_tcpServer( 0 ), m_logInstance( logInstance ), m_ip( ip ), m_port( port )
  {
    m_tcpServer = new ConnectionTCPServer( this, m_logInstance, m_ip, m_port );
  }

  SOCKS5BytestreamServer::~SOCKS5BytestreamServer()
  {
    if( m_tcpServer )
      delete m_tcpServer;

    ConnectionMap::const_iterator it = m_connections.begin();
    for( ; it != m_connections.end(); ++it )
      delete (*it).first;
  }

  ConnectionError SOCKS5BytestreamServer::listen()
  {
    if( m_tcpServer )
      return m_tcpServer->connect();

    return ConnNotConnected;
  }

  ConnectionError SOCKS5BytestreamServer::recv( int timeout )
  {
    if( !m_tcpServer )
      return ConnNotConnected;

    ConnectionError ce = m_tcpServer->recv( timeout );
    if( ce != ConnNoError )
      return ce;

    ConnectionMap::const_iterator it = m_connections.begin();
    ConnectionMap::const_iterator it2;
    while( it != m_connections.end() )
    {
      it2 = it++;
      (*it2).first->recv( timeout );
    }

    util::clearList( m_oldConnections );
    return ConnNoError;
  }

  void SOCKS5BytestreamServer::stop()
  {
    if( m_tcpServer )
    {
      m_tcpServer->disconnect();
      m_tcpServer->cleanup();
    }
  }

  int SOCKS5BytestreamServer::localPort() const
  {
    if( m_tcpServer )
      return m_tcpServer->localPort();

    return m_port;
  }

  const std::string SOCKS5BytestreamServer::localInterface() const
  {
    if( m_tcpServer )
      return m_tcpServer->localInterface();

    return m_ip;
  }

  ConnectionBase* SOCKS5BytestreamServer::getConnection( const std::string& hash )
  {
    util::MutexGuard mg( m_mutex );

    ConnectionMap::iterator it = m_connections.begin();
    for( ; it != m_connections.end(); ++it )
    {
      if( (*it).second.hash == hash )
      {
        ConnectionBase* conn = (*it).first;
        conn->registerConnectionDataHandler( 0 );
        m_connections.erase( it );
        return conn;
      }
    }

    return 0;
  }

  void SOCKS5BytestreamServer::registerHash( const std::string& hash )
  {
    util::MutexGuard mg( m_mutex );
    m_hashes.push_back( hash );
  }

  void SOCKS5BytestreamServer::removeHash( const std::string& hash )
  {
    util::MutexGuard mg( m_mutex );
    m_hashes.remove( hash );
  }

  void SOCKS5BytestreamServer::handleIncomingConnection( ConnectionBase* /*server*/, ConnectionBase* connection )
  {
    connection->registerConnectionDataHandler( this );
    ConnectionInfo ci;
    ci.state = StateUnnegotiated;
    m_connections[connection] = ci;
  }

  void SOCKS5BytestreamServer::handleReceivedData( const ConnectionBase* connection,
                                                   const std::string& data )
  {
    ConnectionMap::iterator it = m_connections.find( const_cast<ConnectionBase*>( connection ) );
    if( it == m_connections.end() )
      return;

    switch( (*it).second.state )
    {
      case StateDisconnected:
        (*it).first->disconnect();
        break;
      case StateUnnegotiated:
      {
        char c[2];
        c[0] = 0x05;
        c[1] = (char)(unsigned char)0xFF;
        (*it).second.state = StateDisconnected;

        if( data.length() >= 3 && data[0] == 0x05 )
        {
          unsigned int sz = ( data.length() - 2 < static_cast<unsigned int>( data[1] ) )
                              ? static_cast<unsigned int>( data.length() - 2 )
                              : static_cast<unsigned int>( data[1] );
          for( unsigned int i = 2; i < sz + 2; ++i )
          {
            if( data[i] == 0x00 )
            {
              c[1] = 0x00;
              (*it).second.state = StateAuthAccepted;
              break;
            }
          }
        }
        (*it).first->send( std::string( c, 2 ) );
        break;
      }
      case StateAuthmethodAccepted:
        // place to implement any future auth support
        break;
      case StateAuthAccepted:
      {
        std::string reply = data;
        if( reply.length() < 2 )
          reply.resize( 2 );

        reply[0] = 0x05;
        reply[1] = 0x01; // general SOCKS server failure
        (*it).second.state = StateDisconnected;

        if( data.length() == 47 && data[0] == 0x05 && data[1] == 0x01 && data[2] == 0x00
            && data[3] == 0x03 && data[4] == 0x28 && data[45] == 0x00 && data[46] == 0x00 )
        {
          const std::string hash = data.substr( 5, 40 );

          HashMap::const_iterator ith = m_hashes.begin();
          for( ; ith != m_hashes.end() && (*ith) != hash; ++ith )
            ;

          if( ith != m_hashes.end() )
          {
            reply[1] = 0x00;
            (*it).second.hash = hash;
            (*it).second.state = StateDestinationAccepted;
          }
        }
        (*it).first->send( reply );
        break;
      }
      case StateDestinationAccepted:
      case StateActive:
        // should not happen
        break;
    }
  }

  void SOCKS5BytestreamServer::handleConnect( const ConnectionBase* /*connection*/ )
  {
    // should never happen, TCP connection is already established
  }

  void SOCKS5BytestreamServer::handleDisconnect( const ConnectionBase* connection,
                                                       ConnectionError /*reason*/ )
  {
    m_connections.erase( const_cast<ConnectionBase*>( connection ) );
    m_oldConnections.push_back( connection );
  }

}
