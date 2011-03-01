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

#include "connectiontcpbase.h"
#include "dns.h"
#include "logsink.h"
#include "prep.h"
#include "mutexguard.h"

#ifdef __MINGW32__
# include <winsock.h>
#endif

#if ( !defined( _WIN32 ) && !defined( _WIN32_WCE ) ) || defined( __SYMBIAN32__ )
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <unistd.h>
#elif ( defined( _WIN32 ) || defined( _WIN32_WCE ) ) && !defined( __SYMBIAN32__ )
# include <winsock.h>
typedef int socklen_t;
#endif

#include <ctime>

#include <cstdlib>
#include <string>

namespace gloox
{

  ConnectionTCPBase::ConnectionTCPBase( const LogSink& logInstance,
                                        const std::string& server, int port )
    : ConnectionBase( 0 ),
      m_logInstance( logInstance ), m_buf( 0 ), m_socket( -1 ), m_totalBytesIn( 0 ),
      m_totalBytesOut( 0 ), m_bufsize( 1024 ), m_cancel( true )
  {
    init( server, port );
  }

  ConnectionTCPBase::ConnectionTCPBase( ConnectionDataHandler* cdh, const LogSink& logInstance,
                                        const std::string& server, int port )
    : ConnectionBase( cdh ),
      m_logInstance( logInstance ), m_buf( 0 ), m_socket( -1 ), m_totalBytesIn( 0 ),
      m_totalBytesOut( 0 ), m_bufsize( 1024 ), m_cancel( true )
  {
    init( server, port );
  }

  void ConnectionTCPBase::init( const std::string& server, int port )
  {
// FIXME check return value?
    prep::idna( server, m_server );
    m_port = port;
    m_buf = (char*)calloc( m_bufsize + 1, sizeof( char ) );
  }

  ConnectionTCPBase::~ConnectionTCPBase()
  {
    cleanup();
    free( m_buf );
    m_buf = 0;
  }

  void ConnectionTCPBase::disconnect()
  {
    util::MutexGuard rm( m_recvMutex );
    m_cancel = true;
  }

  bool ConnectionTCPBase::dataAvailable( int timeout )
  {
    if( m_socket < 0 )
      return true; // let recv() catch the closed fd

    fd_set fds;
    struct timeval tv;

    FD_ZERO( &fds );
    // the following causes a C4127 warning in VC++ Express 2008 and possibly other versions.
    // however, the reason for the warning can't be fixed in gloox.
    FD_SET( m_socket, &fds );

    tv.tv_sec = timeout / 1000000;
    tv.tv_usec = timeout % 1000000;

    return ( ( select( m_socket + 1, &fds, 0, 0, timeout == -1 ? 0 : &tv ) > 0 )
             && FD_ISSET( m_socket, &fds ) != 0 );
  }

  ConnectionError ConnectionTCPBase::receive()
  {
    if( m_socket < 0 )
      return ConnNotConnected;

    ConnectionError err = ConnNoError;
    while( !m_cancel && ( err = recv( 10 ) ) == ConnNoError )
      ;
    return err == ConnNoError ? ConnNotConnected : err;
  }

  bool ConnectionTCPBase::send( const std::string& data )
  {
    m_sendMutex.lock();

    if( data.empty() || ( m_socket < 0 ) )
    {
      m_sendMutex.unlock();
      return false;
    }

    int sent = 0;
    for( size_t num = 0, len = data.length(); sent != -1 && num < len; num += sent )
    {
      sent = static_cast<int>( ::send( m_socket, (data.c_str()+num), (int)(len - num), 0 ) );
    }

    m_totalBytesOut += (int)data.length();

    m_sendMutex.unlock();

    if( sent == -1 && m_handler )
      m_handler->handleDisconnect( this, ConnIoError );

    return sent != -1;
  }

  void ConnectionTCPBase::getStatistics( long int &totalIn, long int &totalOut )
  {
    totalIn = m_totalBytesIn;
    totalOut = m_totalBytesOut;
  }

  void ConnectionTCPBase::cleanup()
  {
    if( !m_sendMutex.trylock() )
      return;

    if( !m_recvMutex.trylock() )
    {
      m_sendMutex.unlock();
      return;
    }

    if( m_socket >= 0 )
    {
      DNS::closeSocket( m_socket, m_logInstance );
      m_socket = -1;
    }

    m_state = StateDisconnected;
    m_cancel = true;
    m_totalBytesIn = 0;
    m_totalBytesOut = 0;

    m_recvMutex.unlock(),
    m_sendMutex.unlock();
  }

  int ConnectionTCPBase::localPort() const
  {
    struct sockaddr local;
    socklen_t len = (socklen_t)sizeof( local );
    if( getsockname ( m_socket, &local, &len ) < 0 )
      return -1;
    else
      return ntohs( ((struct sockaddr_in *)&local)->sin_port );
  }

  const std::string ConnectionTCPBase::localInterface() const
  {
    struct sockaddr_in local;
    socklen_t len = (socklen_t)sizeof( local );
    if( getsockname ( m_socket, (reinterpret_cast<struct sockaddr*>( &local )), &len ) < 0 )
      return EmptyString;
    else
    {
//       char addr[INET_ADDRSTRLEN];
//       return inet_ntop( AF_INET, &(local.sin_addr), addr, sizeof( addr ) ); //FIXME is this portable?
      return inet_ntoa( local.sin_addr );
    }
  }

}
