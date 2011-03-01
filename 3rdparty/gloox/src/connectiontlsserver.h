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

#ifndef CONNECTIONTLSSERVER_H__
#define CONNECTIONTLSSERVER_H__

#include "macros.h"
#include "logsink.h"
#include "connectionbase.h"
#include "connectiontls.h"
#include "tlsdefault.h"
#include "tlshandler.h"

#include <string>

namespace gloox
{

  class ConnectionDataHandler;

  /**
   * @brief This is an implementation of the server-side of a TLS/SSL connection.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API ConnectionTLSServer : public ConnectionTLS
  {
    public:
      /**
       * Constructs a new ConnectionTLSServer object.
       * @param cdh The ConnectionDataHandler that will be notified of events from this connection
       * @param conn A transport connection. It should be an established connection from
       * a client that is about to perform a TLS handshake.
       * ConnectionTLSServer will own the transport connection and delete it in its destructor.
       * @param log The log target. Obtain it from ClientBase::logInstance().
       */
      ConnectionTLSServer( ConnectionDataHandler* cdh, ConnectionBase* conn, const LogSink& log );

      /**
       * Constructs a new ConnectionTLSServer object.
       * @param conn A transport connection. It should be an established connection from
       * a client that is about to perform a TLS handshake.
       * ConnectionTLSServer will own the transport connection and delete it in its destructor.
       * @param log The log target. Obtain it from ClientBase::logInstance().
       */
      ConnectionTLSServer( ConnectionBase* conn, const LogSink& log );

      /**
       * Virtual Destructor.
       */
      virtual ~ConnectionTLSServer();

      /**
       * Returns a TLS server.
       * @return A  TLS server.
       */
      virtual TLSBase* getTLSBase( TLSHandler* th, const std::string server );

      // reimplemented from ConnectionTLS
      virtual ConnectionBase* newInstance() const;

    private:
      ConnectionTLSServer& operator=( const ConnectionTLSServer& );

  };

}

#endif // CONNECTIONTLSSERVER_H__
