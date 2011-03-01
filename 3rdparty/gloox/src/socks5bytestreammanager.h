/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SOCKS5BYTESTREAMMANAGER_H__
#define SOCKS5BYTESTREAMMANAGER_H__

#include "iqhandler.h"
#include "jid.h"
#include "stanzaextension.h"

namespace gloox
{

  class BytestreamHandler;
  class SOCKS5BytestreamServer;
  class SOCKS5Bytestream;
  class ClientBase;

  /**
   * Describes a single StreamHost.
   */
  struct StreamHost
  {
    JID jid;                    /**< The StreamHost's JID. */
    std::string host;           /**< The StreamHost's IP or host name. */
    int port;                   /**< The StreamHost's port. */
//         std::string zeroconf;       /**< A zeroconf identifier. */
  };

  /**
   * A list of StreamHosts.
   */
  typedef std::list<StreamHost> StreamHostList;

  /**
   * @brief An SOCKS5BytestreamManager dispatches SOCKS5 Bytestreams.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SOCKS5BytestreamManager : public IqHandler
  {

    friend class SOCKS5Bytestream;

    public:

      /**
       * Supported transport layer protocols.
       */
      enum S5BMode
      {
        S5BTCP,                     /**< Use TCP on the transport layer. */
        S5BUDP,                     /**< Use UDP on the transport layer. Not currently supported. */
        S5BInvalid                  /**< Invalid mode. */
      };

      /**
       * Constructs a new SOCKS5BytestreamManager.
       * @param parent The ClientBase to use for sending data.
       * @param s5bh A SOCKS5BytestreamManager -derived object that will receive
       * incoming and outgoing SOCKS5Bytestreams.
       */
      SOCKS5BytestreamManager( ClientBase* parent, BytestreamHandler* s5bh );

      /**
       * Virtual destructor.
       */
      virtual ~SOCKS5BytestreamManager();

      /**
       * Sets a list of StreamHosts that will be used for subsequent bytestream requests.
       * @note At least one StreamHost is required.
       * @param hosts A list of StreamHosts.
       */
      void setStreamHosts( StreamHostList hosts ) { m_hosts = hosts; }

      /**
       * Adds one StreamHost to the list of StreamHosts.
       * @param jid The StreamHost's JID.
       * @param host The StreamHost's hostname.
       * @param port The StreamHost's port.
       */
      void addStreamHost( const JID& jid, const std::string& host, int port );

      /**
       * This function requests a bytestream with the remote entity.
       * Data can only be sent over an open stream. Use isOpen() to find out what the stream's
       * current state is. However, successful opening/initiation will be announced by means of the
       * BytestreamHandler interface. Multiple bytestreams (even per JID) can be initiated
       * without waiting for success.
       * @param to The recipient of the requested bytestream.
       * @param mode The desired transport layer protocol.
       * @param sid The bytestream's stream ID, if previously negotiated e.g. using SI (XEP-0095).
       * @param from An optional 'from' address to stamp outgoing
       * requests with. Only useful in component scenarios. Defaults to empty JID.
       * @return @b False in case of an error, @b true otherwise. A return value of @b true does
       * @b not indicate that the bytestream has been opened. This is announced by means of the
       * BytestreamHandler.
       */
      bool requestSOCKS5Bytestream( const JID& to, S5BMode mode, const std::string& sid = EmptyString,
                                    const JID& from = JID() );

      /**
       * To get rid of a bytestream (i.e., close and delete it), call this function. You
       * should then not use the bytestream any more.
       * The remote entity will be notified of the closing of the stream.
       * @param s5b The bytestream to dispose. It will be deleted here.
       */
      bool dispose( SOCKS5Bytestream* s5b );

      /**
       * Use this function to accept an incoming bytestream.
       * @param sid The stream's id as passed to BytestreamHandler::handleIncomingSOCKS5Bytestream().
       */
      void acceptSOCKS5Bytestream( const std::string& sid );

      /**
       * Use this function to reject an incoming bytestream.
       * @param sid The stream's id as passed to BytestreamHandler::handleIncomingSOCKS5Bytestream().
       * @param reason The reason for the reject.
       */
      void rejectSOCKS5Bytestream( const std::string& sid, StanzaError reason = StanzaErrorNotAcceptable );

      /**
       * Use this function to register an object that will receive new @b incoming bytestream
       * requests from the SOCKS5BytestreamManager. Only one BytestreamHandler can be
       * registered at any one time.
       * @param s5bh The BytestreamHandler derived object to receive notifications.
       */
      void registerBytestreamHandler( BytestreamHandler* s5bh )
        { m_socks5BytestreamHandler = s5bh; }

      /**
       * Removes the registered BytestreamHandler.
       */
      void removeBytestreamHandler()
        { m_socks5BytestreamHandler = 0; }

      /**
       * Tells the SOCKS5BytestreamManager which SOCKS5BytestreamServer handles peer-2-peer SOCKS5
       * bytestreams.
       * @param server The SOCKS5BytestreamServer to use.
       */
      void registerSOCKS5BytestreamServer( SOCKS5BytestreamServer* server ) { m_server = server; }

      /**
       * Un-registers any local SOCKS5BytestreamServer.
       */
      void removeSOCKS5BytestreamServer() { m_server = 0; }

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:
#ifdef SOCKS5BYTESTREAMMANAGER_TEST
    public:
#endif

      class Query : public StanzaExtension
      {
        public:
          /**
           * Constructs a new empty Query object.
           */
          Query();

          /**
           * Constructs a new Query (streamhost) object from the given parameters.
           * @param sid The stream ID.
           * @param mode The stream mode (TCP or UDP).
           * @param hosts A list of stream hosts.
           */
          Query( const std::string& sid, S5BMode mode,
                 const StreamHostList& hosts );

          /**
           * Constructs a new Query (streamhost-used or activate) object, including the given JID.
           * @param jid The JID.
           * @param sid The stream ID.
           * @param activate Determines whether the object will be an &apos;activate&apos; (@b true) or
           * &apos;streamhost-used&apos; (@b false) one.
           */
          Query( const JID& jid, const std::string& sid, bool activate );

          /**
           * Constructs a new Query object from the given Tag.
           * @param tag The Tag to parse.
           */
          Query( const Tag* tag );

          /**
           * Virtual destructor.
           */
          virtual ~Query();

          /**
           * Returns the current stream ID.
           * @return The current stream ID.
           */
          const std::string& sid() const { return m_sid; }

          /**
           * Returns the current JID.
           * @return The current JID.
           */
          const JID& jid() const { return m_jid; }

          /**
           * Returns the current mode.
           * @return The current mode.
           */
          S5BMode mode() const { return m_mode; }

          /**
           * Returns the current list of stream hosts.
           * @return The current list of stream hosts.
           */
          const StreamHostList& hosts() const { return m_hosts; }

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Query( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            return new Query( *this );
          }

        private:
          enum QueryType
          {
            TypeSH,
            TypeSHU,
            TypeA,
            TypeInvalid
          };

          std::string m_sid;
          JID m_jid;
          SOCKS5BytestreamManager::S5BMode m_mode;
          StreamHostList m_hosts;
          QueryType m_type;

      };

      SOCKS5BytestreamManager& operator=( const SOCKS5BytestreamManager&);
      void rejectSOCKS5Bytestream( const JID& from, const std::string& id, StanzaError reason = StanzaErrorNotAcceptable );
      bool haveStream( const JID& from );
      const StreamHost* findProxy( const JID& from, const std::string& hostjid, const std::string& sid );

      void acknowledgeStreamHost( bool success, const JID& jid, const std::string& sid );

      enum IBBActionType
      {
        S5BOpenStream,
        S5BCloseStream,
        S5BActivateStream
      };

      typedef std::map<std::string, SOCKS5Bytestream*> S5BMap;
      S5BMap m_s5bMap;

      struct AsyncS5BItem
      {
        JID from;
        JID to;
        std::string id;
        StreamHostList sHosts;
        bool incoming;
      };
      typedef std::map<std::string, AsyncS5BItem> AsyncTrackMap;
      AsyncTrackMap m_asyncTrackMap;

      ClientBase* m_parent;
      BytestreamHandler* m_socks5BytestreamHandler;
      SOCKS5BytestreamServer* m_server;
      StreamHostList m_hosts;
      StringMap m_trackMap;

  };

}

#endif // SOCKS5BYTESTREAMMANAGER_H__
