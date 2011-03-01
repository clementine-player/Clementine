/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SOCKS5BYTESTREAM_H__
#define SOCKS5BYTESTREAM_H__

#include "bytestream.h"
#include "gloox.h"
#include "socks5bytestreammanager.h"
#include "connectiondatahandler.h"

#include <string>

namespace gloox
{

  class SOCKS5BytestreamDataHandler;
  class ConnectionBase;
  class LogSink;

  /**
   * @brief An implementation of a single SOCKS5 Bytestream (XEP-0065).
   *
   * One instance of this class handles one bytestream.
   *
   * See SOCKS5BytestreamManager for a detailed description on how to implement
   * SOCKS5 Bytestreams in your application.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SOCKS5Bytestream : public ConnectionDataHandler, public Bytestream
  {
    friend class SOCKS5BytestreamManager;

    public:
      /**
       * Virtual destructor.
       */
      virtual ~SOCKS5Bytestream();

      /**
       * This function starts the connection process. That is, it attempts to connect
       * to each of the available StreamHosts. Once a working StreamHosts is found, the
       * SOCKS5BytestreamManager is notified and the function returns.
       * @return @b True if a connection to a StreamHost could be established, @b false
       * otherwise.
       * @note If @b false is returned you should hand this SOCKS5Bytestream object
       * to SOCKS5BytestreamManager::dispose() for deletion.
       * @note Make sure you have a SOCKS5BytestreamDataHandler registered (using
       * registerSOCKS5BytestreamDataHandler()) before calling this function.
       */
      virtual bool connect();

      /**
       * Closes the bytestream.
       */
      virtual void close();

      /**
       * Use this function to send a chunk of data over an open bytestream. There is
       * no limit for the size of the chunk (other than your machine's memory).
       * If the stream is not open or has been closed again
       * (by the remote entity or locally), nothing is sent and @b false is returned.
       * @param data The block of data to send.
       * @return @b True if the data has been sent (no guarantee of receipt), @b false
       * in case of an error.
       */
      virtual bool send( const std::string& data );

      /**
       * Call this function repeatedly to receive data from the socket. You should even do this
       * if you use the bytestream to merely @b send data.
       * @param timeout The timeout to use for select in microseconds. Default of -1 means blocking.
       * @return The state of the connection.
       */
      virtual ConnectionError recv( int timeout = -1 );

      /**
       * Sets the connection to use.
       * @param connection The connection. The bytestream will own the connection, any
       * previously set connection gets deleted.
       */
      void setConnectionImpl( ConnectionBase* connection );

      /**
       * This function returns the concrete connection implementation currently in use.
       * @return The concrete connection implementation.
       * @since 0.9.7
       */
      ConnectionBase* connectionImpl( ) { return m_connection; }

      /**
       * Use this function to set the available StreamHosts. Usually you should not need to
       * use this function directly.
       */
      void setStreamHosts( const StreamHostList& hosts ) { m_hosts = hosts; }

      // reimplemented from ConnectionDataHandler
      virtual void handleReceivedData( const ConnectionBase* connection, const std::string& data );

      // reimplemented from ConnectionDataHandler
      virtual void handleConnect( const ConnectionBase* connection );

      // reimplemented from ConnectionDataHandler
      virtual void handleDisconnect( const ConnectionBase* connection, ConnectionError reason );

    private:
      SOCKS5Bytestream( SOCKS5BytestreamManager* manager, ConnectionBase* connection,
                        LogSink& logInstance, const JID& initiator, const JID& target,
                        const std::string& sid );
      void activate();

      SOCKS5BytestreamManager* m_manager;
      ConnectionBase* m_connection;
      ConnectionBase* m_socks5;
      JID m_proxy;
      bool m_connected;

      StreamHostList m_hosts;

  };

}

#endif // SOCKS5BYTESTREAM_H__
