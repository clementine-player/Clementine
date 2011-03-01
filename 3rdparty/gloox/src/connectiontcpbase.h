/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CONNECTIONTCPBASE_H__
#define CONNECTIONTCPBASE_H__

#include "gloox.h"
#include "connectionbase.h"
#include "logsink.h"
#include "mutex.h"

#include <string>

namespace gloox
{

  namespace util
  {
    class Mutex;
  }

  /**
   * @brief This is a base class for a simple TCP connection.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API ConnectionTCPBase : public ConnectionBase
  {
    public:
      /**
       * Constructs a new ConnectionTCPBase object.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param server A server to connect to.
       * @param port The port to connect to. The default of -1 means that XMPP SRV records
       * will be used to find out about the actual host:port.
       * @note To properly use this object, you have to set a ConnectionDataHandler using
       * registerConnectionDataHandler(). This is not necessary if this object is
       * part of a 'connection chain', e.g. with ConnectionHTTPProxy.
       */
      ConnectionTCPBase( const LogSink& logInstance, const std::string& server, int port = -1 );

      /**
       * Constructs a new ConnectionTCPBase object.
       * @param cdh An ConnectionDataHandler-derived object that will handle incoming data.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param server A server to connect to.
       * @param port The port to connect to. The default of -1 means that SRV records will be used
       * to find out about the actual host:port.
       */
      ConnectionTCPBase( ConnectionDataHandler* cdh, const LogSink& logInstance,
                         const std::string& server, int port = -1 );

      /**
       * Virtual destructor
       */
      virtual ~ConnectionTCPBase();

      // reimplemented from ConnectionBase
      virtual bool send( const std::string& data );

      // reimplemented from ConnectionBase
      virtual ConnectionError receive();

      // reimplemented from ConnectionBase
      virtual void disconnect();

      // reimplemented from ConnectionBase
      virtual void cleanup();

      // reimplemented from ConnectionBase
      virtual void getStatistics( long int &totalIn, long int &totalOut );

      /**
       * Gives access to the raw socket of this connection. Use it wisely. You can
       * select()/poll() it and use ConnectionTCPBase::recv( -1 ) to fetch the data.
       * @return The socket of the active connection, or -1 if no connection is established.
       */
      int socket() const { return m_socket; }

      /**
       * This function allows to set an existing socket with an established
       * connection to use in this connection. You will still need to call connect() in order to
       * negotiate the XMPP stream. You should not set a new socket after having called connect().
       * @param socket The existing socket.
       */
      void setSocket( int socket ) { m_cancel = false; m_state = StateConnected; m_socket = socket; }

      /**
       * Returns the local port.
       * @return The local port.
       */
      virtual int localPort() const;

      /**
       * Returns the locally bound IP address.
       * @return The locally bound IP address.
       */
      virtual const std::string localInterface() const;

    protected:
      ConnectionTCPBase& operator=( const ConnectionTCPBase& );
      void init( const std::string& server, int port );
      bool dataAvailable( int timeout = -1 );
      void cancel();

      const LogSink& m_logInstance;
      util::Mutex m_sendMutex;
      util::Mutex m_recvMutex;

      char* m_buf;
      int m_socket;
      long int m_totalBytesIn;
      long int m_totalBytesOut;
      const int m_bufsize;
      bool m_cancel;

  };

}

#endif // CONNECTIONTCPBASE_H__
