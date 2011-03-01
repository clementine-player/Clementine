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

#include "connectiontcpserver.h"
#include "connectiontcpclient.h"
#include "connectionhandler.h"
#include "dns.h"
#include "logsink.h"
#include "mutex.h"
#include "mutexguard.h"
#include "util.h"

#ifdef __MINGW32__
# include <winsock.h>
#endif

#if ( !defined( _WIN32 ) && !defined( _WIN32_WCE ) ) || defined( __SYMBIAN32__ )
# include <netinet/in.h>
# include <arpa/nameser.h>
# include <resolv.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <sys/select.h>
# include <unistd.h>
# include <errno.h>
#endif

#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
# include <winsock.h>
#elif defined( _WIN32_WCE )
# include <winsock2.h>
#endif

#include <cstdlib>
#include <string>

#ifndef _WIN32_WCE
# include <sys/types.h>
#endif

namespace gloox
{

  ConnectionTCPServer::ConnectionTCPServer( ConnectionHandler* ch, const LogSink& logInstance,
                                            const std::string& ip, int port )
    : ConnectionTCPBase( 0, logInstance, ip, port ),
      m_connectionHandler( ch )
  {
  }

  ConnectionTCPServer::~ConnectionTCPServer()
  {
  }

  ConnectionBase* ConnectionTCPServer::newInstance() const
  {
    return new ConnectionTCPServer( m_connectionHandler, m_logInstance, m_server, m_port );
  }

  ConnectionError ConnectionTCPServer::connect()
  {
    util::MutexGuard mg( &m_sendMutex );

    if( m_socket >= 0 || m_state > StateDisconnected )
      return ConnNoError;

    m_state = StateConnecting;

    if( m_socket < 0 )
      m_socket = DNS::getSocket( m_logInstance );

    if( m_socket < 0 )
      return ConnIoError;

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = static_cast<unsigned short int>( htons( m_port ) );
    local.sin_addr.s_addr = m_server.empty() ? INADDR_ANY : inet_addr( m_server.c_str() );
    memset( local.sin_zero, '\0', 8 );

    if( bind( m_socket, (struct sockaddr*)&local, sizeof( struct sockaddr ) ) < 0 )
    {
      std::string message = "bind() to " + ( m_server.empty() ? std::string( "*" ) : m_server )
          + " (" + inet_ntoa( local.sin_addr ) + ":" + util::int2string( m_port ) + ") failed. "
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
          "WSAGetLastError: " + util::int2string( ::WSAGetLastError() );
#else
          "errno: " + util::int2string( errno );
#endif
      m_logInstance.dbg( LogAreaClassConnectionTCPServer, message );

      return ConnIoError;
    }

    if( listen( m_socket, 10 ) < 0 )
    {
      std::string message = "listen on " + ( m_server.empty() ? std::string( "*" ) : m_server )
          + " (" + inet_ntoa( local.sin_addr ) + ":" + util::int2string( m_port ) + ") failed. "
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
          "WSAGetLastError: " + util::int2string( ::WSAGetLastError() );
#else
          "errno: " + util::int2string( errno );
#endif
      m_logInstance.dbg( LogAreaClassConnectionTCPServer, message );

      return ConnIoError;
    }

    m_cancel = false;
    return ConnNoError;
  }

  ConnectionError ConnectionTCPServer::recv( int timeout )
  {
    m_recvMutex.lock();

    if( m_cancel || m_socket < 0 || !m_connectionHandler )
    {
      m_recvMutex.unlock();
      return ConnNotConnected;
    }

    if( !dataAvailable( timeout ) )
    {
      m_recvMutex.unlock();
      return ConnNoError;
    }

    struct sockaddr_in they;
    int sin_size = sizeof( struct sockaddr_in );
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
    int newfd = static_cast<int>( accept( static_cast<SOCKET>( m_socket ), (struct sockaddr*)&they, &sin_size ) );
#else
    int newfd = accept( m_socket, (struct sockaddr*)&they, (socklen_t*)&sin_size );
#endif

    m_recvMutex.unlock();

    ConnectionTCPClient* conn = new ConnectionTCPClient( m_logInstance, inet_ntoa( they.sin_addr ),
                                                         ntohs( they.sin_port ) );
    conn->setSocket( newfd );
    m_connectionHandler->handleIncomingConnection( this, conn );

    return ConnNoError;
  }

}
