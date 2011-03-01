/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIPROFILEFT_H__
#define SIPROFILEFT_H__

#include "iqhandler.h"
#include "socks5bytestreammanager.h"
#include "siprofilehandler.h"
#include "sihandler.h"
#include "simanager.h"
#include "bytestreamhandler.h"

#include <string>
#include <map>

namespace gloox
{

  class ClientBase;
  class InBandBytestream;
  class IQ;
  class JID;
  class SIProfileFTHandler;
  class SOCKS5Bytestream;

  /**
   * @brief An implementation of the file transfer SI profile (XEP-0096).
   *
   * An SIProfileFT object acts as a 'plugin' to the SIManager. SIProfileFT
   * manages most of the file transfer functionality. The naming comes from the fact that
   * File Transfer (FT) is a profile of Stream Initiation (SI).
   *
   * Usage:
   *
   * Create a new SIProfileFT object. It needs a ClientBase -derived object (e.g. Client)
   * as well as a SIProfileFTHandler -derived object that will receive file transfer-related events.
   * If you already use SI and the SIManager somewhere else, you should pass a pointer to that
   * SIManager object as third parameter to SIProfileFT's constructor.
   * @code
   * class MyFileTransferHandler : public SIProfileFTHandler
   * {
   *   // ...
   * };
   *
   * Client* client = new Client( ... );
   * // ...
   * MyFileTransferHandler* mh = new MyFileTransferHandler( ... );
   *
   * SIProfileFT* ft = new SIProfileFT( client, mh );
   * @endcode
   *
   * You are now, basically, ready to send and receive files.
   *
   * A couple of notes:
   * @li There are two (actually two and a half) possible "techniques" to transfer files
   * using SI. The first is using a peer-to-peer SOCKS5 bytestream, optionally via a
   * (special) SOCKS5 proxy.
   * The second techniques is using an in-band bytestream, i.e. the data is encapsulated in XMPP stanzas
   * and sent through the server.
   *
   * @li To be able to send files using the former method (SOCKS5 bytestreams), you may need
   * access to a SOCKS5 bytestream proxy (called StreamHost). This is especially true if either
   * or both of sender and receiver are behind NATs or are otherwise blocked from establishing
   * direct TCP connections. You should use Disco to query a potential SOCKS5 proxy
   * for its host and port parameters and feed that information into SIProfileFT:
   * @code
   * ft->addStreamHost( JID( "proxy.server.dom" ), "101.102.103.104", 6677 );
   * @endcode
   * You should @b not hard-code this information (esp. host/IP and port) into your app since
   * the proxy may go down occasionally or vanish completely.
   *
   * @li In addition to (or even instead of) using external SOCKS5 proxies, you can use a
   * SOCKS5BytestreamServer object that gloox provides:
   * @code
   * SOCKS5BytestreamServer* server = new SOCKS5BytestreamServer( client->logInstance(), 1234 );
   * if( server->listen() != ConnNoError )
   *   printf( "port in use\n" );
   *
   * ft->addStreamHost( client->jid(), my_ip, 1234 );
   * ft->registerSOCKS5BytestreamServer( server );
   * @endcode
   * This listening server should then be integrated into your mainloop to have its
   * @link gloox::SOCKS5BytestreamServer::recv() recv() @endlink method called from time to time.
   * It is safe to put the server into its own thread.
   *
   * @li When you finally receive a Bytestream via the SIProfileFTHandler, you will need
   * to integrate this bytestream with your mainloop, or put it into a separate thread (if
   * occasional blocking is not acceptable). You will need to call
   * @link gloox::Bytestream::connect() connect() @endlink on that Bytestream. For SOCKS5 bytestreams,
   * this function will try to connect to each of the given StreamHosts and block until it has established
   * a connection with one of them (or until all attempts failed). Further, if you want to receive
   * a file via the bytestream, you will have to call recv() on the object from time to time.
   * For in-band bytestreams, @link gloox::Bytestream::connect() connect() @endlink will send an "open the
   * bytestream" request to the contact.
   *
   * @li For both stream types,
   * @link gloox::BytestreamDataHandler::handleBytestreamOpen() BytestreamDataHandler::handleBytestreamOpen() @endlink
   * will announce the established bytestream. The stream then is ready to send and receive data.
   *
   * @li In general, both types of streams can be handled equally, i.e. there's no need to know whether
   * the underlying stream really is a SOCKS5Bytestream or an InBandBytestream.
   * @link gloox::Bytestream::type() Bytestream::type() @endlink tells anyway. Note, however, that
   * sending large amounts of data using in-band bytestreams may trigger rate limiting in some servers.
   *
   * @li If you e.g. told Client to connect through a @link gloox::ConnectionHTTPProxy HTTP proxy @endlink
   * or a @link gloox::ConnectionSOCKS5Proxy SOCKS5 proxy @endlink, or any other ConnectionBase -derived
   * method, or even chains thereof, SIProfileFT will use the same connection types with the same
   * configuration to connect to the Stream Host/SOCKS5 proxy. If this is inappropriate because you have
   * e.g. a local SOCKS5 proxy inside your local network, use SOCKS5Bytestream::setConnectionImpl() to
   * override the above default connection(s).
   *
   * @li Do @b not delete Bytestream objects manually. Use dispose() instead.
   *
   * @li When using the Client's JID as the first argument to addStreamHost() as in the code snippet
   * above, make sure the JID is actually a full JID. If you let the server pick a resource, the call
   * to Client::jid() needs to be made @b after the connection has been established and authenticated,
   * because only then Client knows its full JID. This is generally a good idea, since the server
   * may choose to change the resource, even if you provided one at login.
   *
   * @li The interal SOCKS5BytestreamServer will obviously not work across NATs.
   *
   * @li Using addStreamHost(), you can add as many potential StreamHosts as you like. However, you
   * should add the best options (e.g. the local SOCKS5BytestreamServer) first.
   *
   * When cleaning up, delete the objectes you created above in the opposite order of
   * creation:
   *
   * @code
   * delete server
   * delete ft;
   * delete client;
   * @endcode
   *
   * For usage examples see src/examples/ft_send.cpp and src/examples/ft_recv.cpp.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIProfileFT : public SIProfileHandler, public SIHandler,
                                public BytestreamHandler, public IqHandler
  {
    public:
      /**
       * Supported stream types.
       */
      enum StreamType
      {
        FTTypeS5B     = 1,          /**< SOCKS5 Bytestreams. */
        FTTypeIBB     = 2,          /**< In-Band Bytestreams. */
        FTTypeOOB     = 4,          /**< Out-of-Band Data. */
        FTTypeAll     = 0xFF        /**< All types. */
      };

      /**
       * Constructor.
       * @param parent The ClientBase to use for signaling.
       * @param sipfth The SIProfileFTHandler to receive events.
       * @param manager An optional SIManager to register with. If this is zero, SIProfileFT
       * will create its own SIManager. You should pass a valid SIManager here if you are
       * already using one with the @c parent ClientBase above.
       * @param s5Manager An optional SOCKS5BytestreamManager to use. If this is zero, SIProfileFT
       * will create its own SOCKS5BytestreamManager. You should pass a valid SOCKS5BytestreamManager
       * here if you are already using one with the @c parent ClientBase above.
       * @note If you passed a SIManager and/or SOCKS5BytestreamManager and/or InBandBytestreamManager
       * to SIProfileFT's constructor, these objects will @b not be deleted on desctruction of SIProfileFT.
       */
      SIProfileFT( ClientBase* parent, SIProfileFTHandler* sipfth, SIManager* manager = 0,
                   SOCKS5BytestreamManager* s5Manager = 0 );

      /**
       * Virtual destructor.
       */
      virtual ~SIProfileFT();

      /**
       * Starts negotiating a file transfer with a remote entity.
       * @param to The entity to send the file to. Must be a full JID.
       * @param name The file's name. Mandatory and must not be empty.
       * @param size The file's size. Mandatory and must be > 0.
       * @param hash The file content's MD5 hash.
       * @param desc A description.
       * @param date The file's last modification date/time. See XEP-0082 for details.
       * @param mimetype The file's mime-type. Defaults to 'binary/octet-stream' if empty.
       * @param streamTypes ORed StreamType that can be used for this transfer.
       * @param from An optional 'from' address to stamp outgoing requests with.
       * Used in component scenario only. Defaults to empty JID.
       * @param sid Optionally specify a stream ID (SID). If empty, one will be generated.
       * @return The requested stream's ID (SID). Empty if conditions above (file name, size)
       * are not met.
       */
      const std::string requestFT( const JID& to, const std::string& name, long size,
                                   const std::string& hash = EmptyString,
                                   const std::string& desc = EmptyString,
                                   const std::string& date = EmptyString,
                                   const std::string& mimetype = EmptyString,
                                   int streamTypes = FTTypeAll,
                                   const JID& from = JID(),
                                   const std::string& sid = EmptyString );

      /**
       * Call this function to accept a file transfer request previously announced by means of
       * @link gloox::SIProfileFTHandler::handleFTRequest() SIProfileFTHandler::handleFTRequest() @endlink.
       * @param to The requestor.
       * @param sid The request's sid, as passed to SIProfileHandler::handleFTRequest().
       * @param type The desired stream type to use for this file transfer. Defaults to
       * SOCKS5 Bytestream. You should not use @c FTTypeAll here.
       * @param from An optional 'from' address to stamp outgoing stanzas with.
       * Used in component scenario only. Defaults to empty JID.
       */
      void acceptFT( const JID& to, const std::string& sid,
                     StreamType type = FTTypeS5B, const JID& from = JID() );

      /**
       * Call this function to decline a FT request previously announced by means of
       * @link gloox::SIProfileFTHandler::handleFTRequest() SIProfileFTHandler::handleFTRequest() @endlink.
       * @param to The requestor.
       * @param sid The request's sid, as passed to SIProfileFTHandler::handleFTRequest().
       * @param reason The reason for the reject.
       * @param text An optional human-readable text explaining the decline.
       */
      void declineFT( const JID& to, const std::string& sid, SIManager::SIError reason,
                      const std::string& text = EmptyString );

      /**
       * Cancels the given bytestream. Most useful for SOCKS5 bytestreams where no proxies could be found.
       * The given Bytestream will be deleted.
       * @param bs The Bytestream to cancel.
       * @note Can also be used with IBB.
       */
      void cancel( Bytestream* bs );

      /**
       * To get rid of a bytestream (i.e., close and delete it), call this function.
       * The remote entity will be notified about the closing of the stream.
       * @param bs The bytestream to dispose. It will be deleted here.
       */
      void dispose( Bytestream* bs );

      /**
       * Registers a handler that will be informed about incoming file transfer
       * requests, i.e. when a remote entity wishes to send a file.
       * @param sipfth A SIProfileFTHandler to register. Only one handler can be registered
       * at any one time.
       */
      void registerSIProfileFTHandler( SIProfileFTHandler* sipfth ) { m_handler = sipfth; }

      /**
       * Removes the previously registered file transfer request handler.
       */
      void removeSIProfileFTHandler() { m_handler = 0; }

      /**
       * Sets a list of StreamHosts that will be used for subsequent SOCKS5 bytestream requests.
       * @note At least one StreamHost is required.
       * @param hosts A list of StreamHosts.
       */
      void setStreamHosts( StreamHostList hosts );

      /**
       * Adds one StreamHost to the list of SOCKS5 StreamHosts.
       * @param jid The StreamHost's JID.
       * @param host The StreamHost's hostname.
       * @param port The StreamHost's port.
       */
      void addStreamHost( const JID& jid, const std::string& host, int port );

      /**
       * Tells the interal SOCKS5BytestreamManager which SOCKS5BytestreamServer handles
       * peer-2-peer SOCKS5 bytestreams.
       * @param server The SOCKS5BytestreamServer to use.
       */
      void registerSOCKS5BytestreamServer( SOCKS5BytestreamServer* server )
        { if( m_socks5Manager ) m_socks5Manager->registerSOCKS5BytestreamServer( server ); }

      /**
       * Un-registers any local SOCKS5BytestreamServer.
       */
      void removeSOCKS5BytestreamServer()
        { if( m_socks5Manager ) m_socks5Manager->removeSOCKS5BytestreamServer(); }

      // reimplemented from SIProfileHandler
      virtual void handleSIRequest( const JID& from, const JID& to, const std::string& id,
                                    const SIManager::SI& si );

      // reimplemented from SIHandler
      virtual void handleSIRequestResult( const JID& from, const JID& to, const std::string& sid,
                                          const SIManager::SI& si );

      // reimplemented from SIHandler
      virtual void handleSIRequestError( const IQ& iq, const std::string& sid );

      // reimplemented from BytestreamHandler
      virtual void handleIncomingBytestreamRequest( const std::string& sid, const JID& from );

      // reimplemented from BytestreamHandler
      virtual void handleIncomingBytestream( Bytestream* bs );

      // reimplemented from BytestreamHandler
      virtual void handleOutgoingBytestream( Bytestream* bs );

      // reimplemented from BytestreamHandler
      virtual void handleBytestreamError( const IQ& iq, const std::string& sid );

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:

      enum TrackEnum
      {
        OOBSent
      };

      ClientBase* m_parent;
      SIManager* m_manager;
      SIProfileFTHandler* m_handler;
      SOCKS5BytestreamManager* m_socks5Manager;
      StreamHostList m_hosts;
      StringMap m_id2sid;
      bool m_delManager;
      bool m_delS5Manager;

  };

}

#endif // SIPROFILEFT_H__
