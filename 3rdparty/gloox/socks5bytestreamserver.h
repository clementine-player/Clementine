/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SOCKS5BYTESTREAMSERVER_H__
#define SOCKS5BYTESTREAMSERVER_H__

#include "macros.h"
#include "connectionhandler.h"
#include "logsink.h"
#include "mutex.h"

namespace gloox
{

  class ConnectionTCPServer;

  /**
   * @brief A server listening for SOCKS5 bytestreams.
   *
   * @note You can use a single SOCKS5BytestreamServer instance with multiple Client objects.
   *
   * @note It is safe to put a SOCKS5BytestreamServer instance into a separate thread.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SOCKS5BytestreamServer : public ConnectionHandler, public ConnectionDataHandler
  {

    friend class SOCKS5BytestreamManager;

    public:
      /**
       * Constructs a new SOCKS5BytestreamServer.
       * @param logInstance A LogSink to use.
       * @param port The local port to listen on.
       * @param ip The local IP to bind to. If empty, the server will listen on all local interfaces.
       */
      SOCKS5BytestreamServer( const LogSink& logInstance, int port, const std::string& ip = EmptyString );

      /**
       * Destructor.
       */
      ~SOCKS5BytestreamServer();

      /**
       * Starts listening on the specified interface and port.
       * @return Returns @c ConnNoError on success, @c ConnIoError on failure (e.g. if the port
       * is already in use).
       */
      ConnectionError listen();

      /**
       * Call this function repeatedly to check for incoming connections and to negotiate
       * them.
       * @param timeout The timeout to use for select in microseconds.
       * @return The state of the listening socket.
       */
      ConnectionError recv( int timeout );

      /**
       * Stops listening and unbinds from the interface and port.
       */
      void stop();

      /**
       * Expose our TCP Connection localPort
       * Returns the local port.
       * @return The local port.
       */
      int localPort() const;

      /**
       * Expose our TCP Connection localInterface
       * Returns the locally bound IP address.
       * @return The locally bound IP address.
       */
      const std::string localInterface() const;

      // reimplemented from ConnectionHandler
      virtual void handleIncomingConnection( ConnectionBase* server, ConnectionBase* connection );

      // reimplemented from ConnectionDataHandler
      virtual void handleReceivedData( const ConnectionBase* connection, const std::string& data );

      // reimplemented from ConnectionDataHandler
      virtual void handleConnect( const ConnectionBase* connection );

      // reimplemented from ConnectionDataHandler
      virtual void handleDisconnect( const ConnectionBase* connection, ConnectionError reason );

    private:
      SOCKS5BytestreamServer& operator=( const SOCKS5BytestreamServer& );
      void registerHash( const std::string& hash );
      void removeHash( const std::string& hash );
      ConnectionBase* getConnection( const std::string& hash );

      enum NegotiationState
      {
        StateDisconnected,
        StateUnnegotiated,
        StateAuthmethodAccepted,
        StateAuthAccepted,
        StateDestinationAccepted,
        StateActive
      };

      struct ConnectionInfo
      {
        NegotiationState state;
        std::string hash;
      };

      typedef std::map<ConnectionBase*, ConnectionInfo> ConnectionMap;
      ConnectionMap m_connections;

      typedef std::list<const ConnectionBase*> ConnectionList;
      ConnectionList m_oldConnections;

      typedef std::list<std::string> HashMap;
      HashMap m_hashes;

      ConnectionTCPServer* m_tcpServer;

      util::Mutex m_mutex;
      const LogSink& m_logInstance;
      std::string m_ip;
      int m_port;

  };

}

#endif // SOCKS5BYTESTREAMSERVER_H__
