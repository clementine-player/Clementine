/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CONNECTIONHTTPPROXY_H__
#define CONNECTIONHTTPPROXY_H__

#include "gloox.h"
#include "connectionbase.h"
#include "logsink.h"

#include <string>

namespace gloox
{

  /**
   * @brief This is an implementation of a simple HTTP Proxying connection.
   *
   * Usage:
   *
   * @code
   * Client* c = new Client( ... );
   * ConnectionTCPClient* conn0 = new ConnectionTCPClient( c->logInstance(),
   *                                                       proxyHost, proxyPort );
   * ConnectionHTTPProxy* conn1 = new ConnectionHTTPProxy( c, conn0, c->logInstance(),
   *                                                       xmppHost, xmppPort );
   * c->setConnectionImpl( conn1 );
   * @endcode
   *
   * Make sure to pass the proxy host/port to the transport connection (ConnectionTCPClient in this case),
   * and the XMPP host/port to the proxy connection.
   *
   * ConnectionHTTPProxy uses the CONNECT method to pass through the proxy. If your proxy does not
   * allow this kind of connections, or if it kills connections after some time, you may want to use
   * ConnectionBOSH instead or in addition.
   *
   * The reason why ConnectionHTTPProxy doesn't manage its own ConnectionTCPClient is that it allows it
   * to be used with other transports (like IPv6 or chained SOCKS5/HTTP proxies).
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API ConnectionHTTPProxy : public ConnectionBase, public ConnectionDataHandler
  {
    public:
      /**
       * Constructs a new ConnectionHTTPProxy object.
       * @param connection A transport connection. It should be configured to connect to
       * the proxy host and port, @b not to the XMPP host. ConnectionHTTPProxy will own the
       * transport connection and delete it in its destructor.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param server A server to connect to. This is the XMPP server's address, @b not the proxy.
       * @param port The port to connect to. This is the XMPP server's port, @b not the proxy's.
       * The default of -1 means that SRV records will be used to find out about the actual host:port.
       * @note To properly use this object, you have to set a ConnectionDataHandler using
       * registerConnectionDataHandler(). This is not necessary if this object is
       * part of a 'connection chain', e.g. with ConnectionSOCKS5Proxy.
       */
      ConnectionHTTPProxy( ConnectionBase* connection, const LogSink& logInstance,
                           const std::string& server, int port = -1 );

      /**
       * Constructs a new ConnectionHTTPProxy object.
       * @param cdh An ConnectionDataHandler-derived object that will handle incoming data.
       * @param connection A transport connection. It should be configured to connect to
       * the proxy host and port, @b not to the XMPP host. ConnectionHTTPProxy will own the
       * transport connection and delete it in its destructor.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param server A server to connect to. This is the XMPP server's address, @b not the proxy.
       * @param port The port to connect to. This is the XMPP server's port, @b not the proxy's.
       * The default of -1 means that SRV records will be used to find out about the actual host:port.
       */
      ConnectionHTTPProxy( ConnectionDataHandler* cdh, ConnectionBase* connection,
                           const LogSink& logInstance,
                           const std::string& server, int port = -1 );

      /**
       * Virtual destructor
       */
      virtual ~ConnectionHTTPProxy();

      // reimplemented from ConnectionBase
      virtual ConnectionError connect();

      // reimplemented from ConnectionBase
      virtual ConnectionError recv( int timeout = -1 );

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

      // reimplemented from ConnectionDataHandler
      virtual void handleReceivedData( const ConnectionBase* connection, const std::string& data );

      // reimplemented from ConnectionDataHandler
      virtual void handleConnect( const ConnectionBase* connection );

      // reimplemented from ConnectionDataHandler
      virtual void handleDisconnect( const ConnectionBase* connection, ConnectionError reason );

      // reimplemented from ConnectionDataHandler
      virtual ConnectionBase* newInstance() const;

      /**
       * Sets the XMPP server to proxy to.
       * @param host The XMPP server hostname (IP address).
       * @param port The XMPP server port. The default of -1 means that SRV records will be used
       * to find out about the actual host:port.
       */
      void setServer( const std::string& host, int port = -1 )
        { m_server = host; m_port = port; }

      /**
       * Sets proxy authorization credentials.
       * @param user The user name to use for proxy authorization.
       * @param password The password to use for proxy authorization.
       */
      void setProxyAuth( const std::string& user, const std::string& password )
        { m_proxyUser = user; m_proxyPwd = password; }

      /**
       * Sets the underlying transport connection. A possibly existing connection will be deleted.
       * @param connection The ConnectionBase to replace the current connection, if any.
       */
      void setConnectionImpl( ConnectionBase* connection );

      /**
       * Switches usage of HTTP/1.1 on or off.
       * @param http11 Set this to @b true to connect through a HTTP/1.1-only proxy, or @b false
       * to use HTTP/1.0. Defaults to HTTP/1.0 which should work with 99.9% of proxies.
       */
      void setHTTP11( bool http11 ) { m_http11 = http11; }

   private:
      ConnectionHTTPProxy &operator=( const ConnectionHTTPProxy& );

      ConnectionBase* m_connection;
      const LogSink& m_logInstance;

      std::string m_proxyUser;
      std::string m_proxyPwd;
      std::string m_proxyHandshakeBuffer;

      bool m_http11;

  };

}

#endif // CONNECTIONHTTPPROXY_H__
