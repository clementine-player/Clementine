/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CONNECTIONTCPCLIENT_H__
#define CONNECTIONTCPCLIENT_H__

#include "gloox.h"
#include "connectiontcpbase.h"
#include "logsink.h"

#include <string>

namespace gloox
{

  /**
   * @brief This is an implementation of a simple TCP connection.
   *
   * You should only need to use this class directly if you need access to some special feature, like
   * the raw socket(), or if you need HTTP proxy support (see @ref gloox::ConnectionHTTPProxy for more
   * information).
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API ConnectionTCPClient : public ConnectionTCPBase
  {
    public:
      /**
       * Constructs a new ConnectionTCPClient object.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param server A server to connect to.
       * @param port The port to connect to. The default of -1 means that XMPP SRV records
       * will be used to find out about the actual host:port.
       * @note To properly use this object, you have to set a ConnectionDataHandler using
       * registerConnectionDataHandler(). This is not necessary if this object is
       * part of a 'connection chain', e.g. with ConnectionHTTPProxy.
       */
      ConnectionTCPClient( const LogSink& logInstance, const std::string& server, int port = -1 );

      /**
       * Constructs a new ConnectionTCPClient object.
       * @param cdh An ConnectionDataHandler-derived object that will handle incoming data.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param server A server to connect to.
       * @param port The port to connect to. The default of -1 means that SRV records will be used
       * to find out about the actual host:port.
       */
      ConnectionTCPClient( ConnectionDataHandler* cdh, const LogSink& logInstance,
                           const std::string& server, int port = -1 );

      /**
       * Virtual destructor
       */
      virtual ~ConnectionTCPClient();

      // reimplemented from ConnectionBase
      virtual ConnectionError recv( int timeout = -1 );

      // reimplemented from ConnectionBase
      virtual ConnectionError connect();

      // reimplemented from ConnectionBase
      virtual ConnectionBase* newInstance() const;

    private:
      ConnectionTCPClient &operator=( const ConnectionTCPClient & );

  };

}

#endif // CONNECTIONTCPCLIENT_H__
