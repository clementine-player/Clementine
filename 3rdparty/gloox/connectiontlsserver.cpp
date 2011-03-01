/*
 * Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
 * This file is part of the gloox library. http://camaya.net/gloox
 *
 * This software is distributed under a license. The full license
 * agreement can be found in the file LICENSE in this distribution.
 * This software may not be copied, modified, sold or distributed
 * other than expressed in the named license agreement.
 *
 * This software is distributed without any warranty.
 */

#include "connectiontlsserver.h"

namespace gloox
{

  ConnectionTLSServer::ConnectionTLSServer( ConnectionDataHandler* cdh, ConnectionBase* conn,
                                            const LogSink& log )
    : ConnectionTLS( cdh, conn, log )
  {
  }

  ConnectionTLSServer::ConnectionTLSServer( ConnectionBase* conn, const LogSink& log )
    : ConnectionTLS( conn, log )
  {
  }

  ConnectionTLSServer::~ConnectionTLSServer()
  {
  }

  TLSBase* ConnectionTLSServer::getTLSBase( TLSHandler* th, const std::string server )
  {
    return new TLSDefault( th, server, TLSDefault::VerifyingServer );
  }

  ConnectionBase* ConnectionTLSServer::newInstance() const
  {
    ConnectionBase* newConn = 0;
    if( m_connection )
      newConn = m_connection->newInstance();
    return new ConnectionTLSServer( m_handler, newConn, m_log );
  }

}
