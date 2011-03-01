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


#include "connectiontls.h"
#include "tlsdefault.h"

namespace gloox
{

  ConnectionTLS::ConnectionTLS( ConnectionDataHandler* cdh, ConnectionBase* conn, const LogSink& log )
    : ConnectionBase( cdh ),
      m_connection( conn ), m_tls( 0 ), m_tlsHandler( 0 ),
      m_log( log )
  {
    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionTLS::ConnectionTLS( ConnectionBase* conn, const LogSink& log )
    : ConnectionBase( 0 ),
      m_connection( conn ), m_tls( 0 ), m_tlsHandler( 0 ), m_log( log )
  {
    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionTLS::~ConnectionTLS()
  {
    delete m_connection;
    delete m_tls;
  }

  void ConnectionTLS::setConnectionImpl( ConnectionBase* connection )
  {
    if( m_connection )
      m_connection->registerConnectionDataHandler( 0 );

    m_connection = connection;

    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionError ConnectionTLS::connect()
  {
    if( !m_connection )
      return ConnNotConnected;

    if( m_state == StateConnected )
      return ConnNoError;

    if( !m_tls )
      m_tls = getTLSBase( this, m_connection->server() );

    if( !m_tls )
      return ConnTlsNotAvailable;

    if( !m_tls->init( m_clientKey, m_clientCerts, m_cacerts ) )
      return ConnTlsFailed;

//     m_tls->setCACerts( m_cacerts );
//     m_tls->setClientCert( m_clientKey, m_clientCerts );

    m_state = StateConnecting;

    if( m_connection->state() != StateConnected )
      return m_connection->connect();

    if( m_tls->handshake() )
      return ConnNoError;
    else
      return ConnTlsFailed;
 }

  ConnectionError ConnectionTLS::recv( int timeout )
  {
    if( m_connection->state() == StateConnected )
    {
      return m_connection->recv( timeout );
    }
    else
    {
      m_log.log( LogLevelWarning, LogAreaClassConnectionTLS,
                 "Attempt to receive data on a connection that is not connected (or is connecting)" );
      return ConnNotConnected;
    }
  }

  bool ConnectionTLS::send( const std::string& data )
  {
    if( m_state != StateConnected )
      return false;

    m_tls->encrypt( data );
    return true;
  }

  ConnectionError ConnectionTLS::receive()
  {
    if( m_connection )
      return m_connection->receive();
    else
      return ConnNotConnected;
  }

  void ConnectionTLS::disconnect()
  {
    if( m_connection )
      m_connection->disconnect();

    cleanup();
  }

  void ConnectionTLS::cleanup()
  {
    if( m_connection )
      m_connection->cleanup();
    if( m_tls )
      m_tls->cleanup();

    m_state = StateDisconnected;
  }

  void ConnectionTLS::getStatistics( long int& totalIn, long int& totalOut )
  {
    if( m_connection )
      m_connection->getStatistics( totalIn, totalOut );
  }

  ConnectionBase* ConnectionTLS::newInstance() const
  {
    ConnectionBase* newConn = 0;
    if( m_connection )
      newConn = m_connection->newInstance();
    return new ConnectionTLS( m_handler, newConn, m_log );
  }

  void ConnectionTLS::handleReceivedData( const ConnectionBase* /*connection*/, const std::string& data )
  {
    if( m_tls )
      m_tls->decrypt( data );
  }

  void ConnectionTLS::handleConnect( const ConnectionBase* /*connection*/ )
  {
    if( m_tls )
      m_tls->handshake();
  }

  void ConnectionTLS::handleDisconnect( const ConnectionBase* /*connection*/, ConnectionError reason )
  {
    if( m_handler )
      m_handler->handleDisconnect( this, reason );

    cleanup();
  }

  void ConnectionTLS::handleEncryptedData( const TLSBase* /*tls*/, const std::string& data )
  {
    if( m_connection )
      m_connection->send( data );
  }

  void ConnectionTLS::handleDecryptedData( const TLSBase* /*tls*/, const std::string& data )
  {
    if( m_handler )
      m_handler->handleReceivedData( this, data );
    else
    {
      m_log.log( LogLevelDebug, LogAreaClassConnectionTLS, "Data received and decrypted but no handler" );
    }
  }

  void ConnectionTLS::handleHandshakeResult( const TLSBase* tls, bool success, CertInfo& certinfo )
  {
    if( success )
    {
      m_state = StateConnected;
      m_log.log( LogLevelDebug, LogAreaClassConnectionTLS, "TLS handshake succeeded" );
      if( m_tlsHandler )
        m_tlsHandler->handleHandshakeResult( tls, success, certinfo );
      if( m_handler )
        m_handler->handleConnect( this );
    }
    else
    {
      m_state = StateDisconnected;
      m_log.log( LogLevelWarning, LogAreaClassConnectionTLS, "TLS handshake failed" );
      if( m_tlsHandler )
        m_tlsHandler->handleHandshakeResult( tls, success, certinfo );
    }
  }

}
