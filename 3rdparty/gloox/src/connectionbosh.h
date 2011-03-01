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

#ifndef CONNECTIONBOSH_H__
#define CONNECTIONBOSH_H__

#include "gloox.h"
#include "connectionbase.h"
#include "logsink.h"
#include "taghandler.h"
#include "parser.h"

#include <string>
#include <list>
#include <ctime>

namespace gloox
{

  /**
   * @brief This is an implementation of a BOSH (HTTP binding) connection.
   *
   * Usage:
   *
   * @code
   * Client *c = new Client( ... );
   * c->setConnectionImpl( new ConnectionBOSH( c,
   *                                new ConnectionTCPClient( c->logInstance(), httpServer, httpPort ),
   *                                c->logInstance(), boshHost, xmpphost, xmppPort ) );
   * @endcode
   *
   * Make sure to pass the BOSH connection manager's host/port to the transport connection
   * (ConnectionTCPClient in this case), and the XMPP server's host and port to the BOSH connection.
   * You must also pass to BOSH the address of the BOSH server you are dealing with, this is used
   * in the HTTP Host header.
   *
   * In the case of using ConnectionBOSH through a HTTP proxy, supply httpServer and httpPort as
   * those of the proxy. In all cases, boshHost should be set to the hostname (not IP address) of
   * the server running the BOSH connection manager.
   *
   * The reason why ConnectionBOSH doesn't manage its own ConnectionTCPClient is that it allows it
   * to be used with other transports (like chained SOCKS5/HTTP proxies, or ConnectionTLS
   * for HTTPS).
   *
   * @note To avoid problems, you should disable TLS in gloox by calling
   * ClientBase::setTls( TLSDisabled ).
   *
   * Sample configurations for different servers can be found in the bosh_example.cpp file included
   * with gloox in the @b src/examples/ directory.
   *
   * @author Matthew Wild <mwild1@gmail.com>
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API ConnectionBOSH : public ConnectionBase, ConnectionDataHandler, TagHandler
  {
    public:
      /**
       * Constructs a new ConnectionBOSH object.
       * @param connection A transport connection. It should be configured to connect to
       * the BOSH connection manager's (or a HTTP proxy's) host and port, @b not to the XMPP host.
       * ConnectionBOSH will own the transport connection and delete it in its destructor.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param boshHost The hostname of the BOSH connection manager
       * @param xmppServer A server to connect to. This is the XMPP server's address, @b not the
       * connection manager's.
       * @param xmppPort The port to connect to. This is the XMPP server's port, @b not the connection
       * manager's.
       * @note To properly use this object, you have to set a ConnectionDataHandler using
       * registerConnectionDataHandler(). This is not necessary if this object is
       * part of a 'connection chain', e.g. with ConnectionSOCKS5Proxy.
       */
      ConnectionBOSH( ConnectionBase* connection, const LogSink& logInstance, const std::string& boshHost,
                      const std::string& xmppServer, int xmppPort = 5222 );

      /**
       * Constructs a new ConnectionBOSH object.
       * @param cdh An ConnectionDataHandler-derived object that will handle incoming data.
       * @param connection A transport connection. It should be configured to connect to
       * the connection manager's (or proxy's) host and port, @b not to the XMPP host. ConnectionBOSH
       * will own the transport connection and delete it in its destructor.
       * @param logInstance The log target. Obtain it from ClientBase::logInstance().
       * @param boshHost The hostname of the BOSH connection manager (not any intermediate proxy)
       * @param xmppServer A server to connect to. This is the XMPP server's address, @b not the connection
       * manager's.
       * @param xmppPort The port to connect to. This is the XMPP server's port, @b not the connection
       * manager's.
       */
      ConnectionBOSH( ConnectionDataHandler* cdh, ConnectionBase* connection,
                      const LogSink& logInstance, const std::string& boshHost,
                      const std::string& xmppServer, int xmppPort = 5222 );

      /**
       * Virtual destructor
       */
      virtual ~ConnectionBOSH();

      /**
       * The supported connection modes. Usually auto-detected.
       */
      enum ConnMode
      {
        ModeLegacyHTTP,             /**< HTTP 1.0 connections, closed after receiving a response */
        ModePersistentHTTP,         /**< HTTP 1.1 connections, re-used after receiving a response */
        ModePipelining              /**< HTTP Pipelining (implies HTTP 1.1) a single connection is used */
      };

      /**
       * Sets the XMPP server to proxy to.
       * @param xmppHost The XMPP server hostname (IP address).
       * @param xmppPort The XMPP server port.
       */
      void setServer( const std::string& xmppHost, unsigned short xmppPort = 5222 )
        { m_server = xmppHost; m_port = xmppPort; }

      /**
       * Sets the path on the connection manager to request
       * @param path The path, the default is "/http-bind/", which is the default for
       * many connection managers.
       */
      void setPath( const std::string& path ) { m_path = path; }

      /**
       * Sets the connection mode
       * @param mode The connection mode, @sa ConnMode
       * @note In the case that a mode is selected that the connection manager
       * or proxy does not support, gloox will fall back to using HTTP/1.0 connections,
       * which should work with any server.
       */
      void setMode( ConnMode mode ) { m_connMode = mode; }

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
      virtual void getStatistics( long int& totalIn, long int& totalOut );

      // reimplemented from ConnectionDataHandler
      virtual void handleReceivedData( const ConnectionBase* connection, const std::string& data );

      // reimplemented from ConnectionDataHandler
      virtual void handleConnect( const ConnectionBase* connection );

      // reimplemented from ConnectionDataHandler
      virtual void handleDisconnect( const ConnectionBase* connection, ConnectionError reason );

      // reimplemented from ConnectionDataHandler
      virtual ConnectionBase* newInstance() const;

      // reimplemented from TagHandler
      virtual void handleTag( Tag* tag );

    private:
      ConnectionBOSH& operator=( const ConnectionBOSH& );
      void initInstance( ConnectionBase* connection, const std::string& xmppServer, const int xmppPort );
      bool sendRequest( const std::string& xml );
      bool sendXML();
      const std::string getHTTPField( const std::string& field );
      ConnectionBase* getConnection();
      ConnectionBase* activateConnection();
      void putConnection();

      //ConnectionBase *m_connection;
      const LogSink& m_logInstance;

      Parser m_parser;   // Used for parsing XML section of responses
      std::string m_boshHost;   // The hostname of the BOSH connection manager
      std::string m_boshedHost;   // The hostname of the BOSH connection manager + : + port
      std::string m_path;   // The path part of the URL that we need to request

      // BOSH parameters
      unsigned long m_rid;
      std::string m_sid;

      bool m_initialStreamSent;
      int m_openRequests;
      int m_maxOpenRequests;
      int m_wait;
      int m_hold;

      bool m_streamRestart;   // Set to true if we are waiting for an acknowledgement of a stream restart

      time_t m_lastRequestTime;
      unsigned long m_minTimePerRequest;

      std::string m_buffer;   // Buffer of received data
      std::string m_bufferHeader;   // HTTP header of data currently in buffer // FIXME doens't need to be member
      std::string::size_type m_bufferContentLength;   // Length of the data in the current response

      std::string m_sendBuffer;   // Data waiting to be sent

      typedef std::list<ConnectionBase*> ConnectionList;
      ConnectionList m_activeConnections;
      ConnectionList m_connectionPool;
      ConnMode m_connMode;

  };

}

#endif // CONNECTIONBOSH_H__
