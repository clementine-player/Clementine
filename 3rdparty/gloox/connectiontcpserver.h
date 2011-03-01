/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CONNECTIONTCPSERVER_H__
#define CONNECTIONTCPSERVER_H__

#include "gloox.h"
#include "connectiontcpbase.h"
#include "logsink.h"

#include <string>

namespace gloox
{

  class ConnectionHandler;

  /**
   * @brief This is an implementation of a simple listening TCP connection.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API ConnectionTCPServer : public ConnectionTCPBase
  {
    public:
      /**
       * Constructs a new ConnectionTCPServer object.
       * @param ch An ConnectionHandler-derived object that will handle incoming connections.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param ip The local IP address to listen on. This must @b not be a hostname.
       * Leave this empty to listen on all local interfaces.
       * @param port The port to listen on.
       */
      ConnectionTCPServer( ConnectionHandler* ch, const LogSink& logInstance,
                           const std::string& ip, int port );

      /**
       * Virtual destructor
       */
      virtual ~ConnectionTCPServer();

      // reimplemented from ConnectionBase
      virtual ConnectionError recv( int timeout = -1 );

      /**
       * This function actually starts @c listening on the port given in the
       * constructor.
       */
      // reimplemented from ConnectionBase
      virtual ConnectionError connect();

      // reimplemented from ConnectionBase
      virtual ConnectionBase* newInstance() const;

    private:
      ConnectionTCPServer &operator=( const ConnectionTCPServer & );

      ConnectionHandler* m_connectionHandler;

  };

}

#endif // CONNECTIONTCPSERVER_H__
