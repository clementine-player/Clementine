/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef CLIENTBASE_H__
#define CLIENTBASE_H__

#include "macros.h"
#include "gloox.h"
#include "eventdispatcher.h"
#include "iqhandler.h"
#include "jid.h"
#include "logsink.h"
#include "mutex.h"
#include "taghandler.h"
#include "statisticshandler.h"
#include "tlshandler.h"
#include "compressiondatahandler.h"
#include "connectiondatahandler.h"
#include "parser.h"

#include <string>
#include <list>
#include <map>

#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
#include <windows.h>
#define SECURITY_WIN32
#include <security.h>
#endif

namespace gloox
{

  class Disco;
  class EventHandler;
  class Event;
  class Tag;
  class IQ;
  class Message;
  class Presence;
  class Subscription;
  class MessageSessionHandler;
  class ConnectionListener;
  class MessageHandler;
  class MessageSession;
  class PresenceHandler;
  class SubscriptionHandler;
  class MUCInvitationHandler;
  class TagHandler;
  class TLSBase;
  class ConnectionBase;
  class CompressionBase;
  class StanzaExtensionFactory;

  /**
   * @brief This is the common base class for a Jabber/XMPP Client and a Jabber Component.
   *
   * It manages connection establishing, authentication, filter registration and invocation.
   * You should normally use Client for client connections and Component for component connections.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API ClientBase : public TagHandler, public ConnectionDataHandler,
                               public CompressionDataHandler, public TLSHandler,
                               public IqHandler
  {

    friend class RosterManager;

    public:
      /**
       * Constructs a new ClientBase.
       * You should not need to use this class directly. Use Client or Component instead.
       * @param ns The namespace which qualifies the stream. Either jabber:client or jabber:component:*
       * @param server The server to connect to.
       * @param port The port to connect to. The default of -1 means to look up the port via DNS SRV
       * or to use a default port of 5222 as defined in XMPP Core.
       */
      ClientBase( const std::string& ns, const std::string& server, int port = -1 );

      /**
       * Constructs a new ClientBase.
       * You should not need to use this class directly. Use Client or Component instead.
       * @param ns The namespace which qualifies the stream. Either jabber:client or jabber:component:*
       * @param password The password to use for further authentication.
       * @param server The server to connect to.
       * @param port The port to connect to. The default of -1 means to look up the port via DNS SRV
       * or to use a default port of 5222 as defined in XMPP: Core.
       */
      ClientBase( const std::string& ns, const std::string& password,
                  const std::string& server, int port = -1 );

      /**
       * Virtual destrcuctor.
       */
      virtual ~ClientBase();

      /**
       * Initiates the connection to a server. This function blocks as long as a connection is
       * established.
       * You can have the connection block 'til the end of the connection, or you can have it return
       * immediately. If you choose the latter, its your responsibility to call @ref recv() every now
       * and then to actually receive data from the socket and to feed the parser.
       * @param block @b True for blocking, @b false for non-blocking connect. Defaults to @b true.
       * @return @b False if prerequisits are not met (server not set) or if the connection was refused,
       * @b true otherwise.
       * @note Since 0.9 @link ConnectionListener::onDisconnect() onDisconnect() @endlink is called
       * in addition to a return value of @b false.
       */
      bool connect( bool block = true );

      /**
       * Use this periodically to receive data from the socket and to feed the parser. You need to use
       * this only if you chose to connect in non-blocking mode.
       * @param timeout The timeout in microseconds to use for select. Default of -1 means blocking
       * until data was available.
       * @return The state of the connection.
       */
      ConnectionError recv( int timeout = -1 );

      /**
       * Reimplement this function to provide a username for connection purposes.
       * @return The username.
       */
      virtual const std::string& username() const { return m_jid.username(); }

      /**
       * Returns the current Jabber ID. If an authorization ID has been set (using setAuthzid())
       * this authzid is returned.
       * @return A reference to the Jabber ID.
       * @note If you change the server part of the JID, the server of the connection is not synced.
       * You have to do that manually using @ref setServer().
       */
      const JID& jid() { return m_authzid ? m_authzid : m_jid; }

      /**
       * Switches usage of SASL on/off. Default: on. SASL should only be disabled if there are
       * problems with using it.
       * @param sasl Whether to switch SASL usage on or off.
       */
      void setSasl( bool sasl ) { m_sasl = sasl; }

      /**
       * Sets the TLS policy. Default: TLS will be used if available. TLS should only be
       * disabled if there are problems with using it.
       * @param tls The TLS policy.
       */
      void setTls( TLSPolicy tls ) { m_tls = tls; }

      /**
       * Switches usage of Stream Compression on/off (if available). Default: on if available. Stream
       * Compression should only be disabled if there are problems with using it.
       * @param compression Whether to switch Stream Compression usage on or off.
       */
      void setCompression( bool compression ) { m_compress = compression; }

      /**
       * Sets the port to connect to. This is not necessary if either the default port (5222) is used
       * or SRV records exist which will be resolved.
       * @param port The port to connect to.
       */
      void setPort( int port ) { m_port = port; }

      /**
       * Sets the XMPP server to connect to.
       * @param server The server to connect to. Either IP or fully qualified domain name.
       * @note If you change the server, the server part of the JID is not synced. You have to do that
       * manually using @ref jid() and @ref JID::setServer().
       * @note This function also sets the server of the Connection(Base) in use.
       */
      void setServer( const std::string &server );

      /**
       * Sets the password to use to connect to the XMPP server.
       * @param password The password to use for authentication.
       */
      void setPassword( const std::string &password ) { m_password = password; }

      /**
       * Returns the current prepped server.
       * @return The server used to connect.
       */
      const std::string& server() const { return m_server; }

      /**
       * Returns whether SASL is currently enabled (not necessarily used).
       * @return The current SASL status.
       */
      bool sasl() const { return m_sasl; }

      /**
       * Returns whether TLS is currently enabled (not necessarily used).
       * @return The current TLS status.
       */
      TLSPolicy tls() const { return m_tls; }

      /**
       * Returns whether Stream Compression is currently enabled (not necessarily used).
       * @return The current Stream Compression status.
       */
      bool compression() const { return m_compress; }

      /**
       * Returns the port. The default of -1 means that the actual port will be looked up using
       * SRV records, or the XMPP default port of 5222 will be used.
       * @return The port used to connect.
       */
      int port() const { return m_port; }

      /**
       * Returns the current password.
       * @return The password used to connect.
       */
      virtual const std::string& password() const { return m_password; }

      /**
       * This function gives access to the @c Disco object.
       * @return A pointer to the Disco object.
       */
      virtual Disco* disco() const { return m_disco; }

      /**
       * Creates a string which is unique in the current instance and
       * can be used as an ID for queries.
       * @return A unique string suitable for query IDs.
       */
      const std::string getID();

      /**
       * Sends the given Tag over an established connection.
       * The ClientBase object becomes the owner of this Tag and will delete it after sending it.
       * You should not rely on the existance of the Tag after it's been sent. If you still need
       * it after sending it, use Tag::clone() to create a deep copy.
       * @param tag The Tag to send.
       */
      void send( Tag* tag );

      /**
       * Sends the given IQ stanza. The given IqHandler is registered to be notified of replies. This,
       * of course, only works for IQs of type get or set. An ID is added if necessary.
       * @param iq The IQ stanza to send.
       * @param ih The handler to register for replies.
       * @param context A value that allows for restoring context.
       * @param del Whether or not delete the IqHandler object after its being called.
       * Default: @b false.
       */
      void send( IQ& iq, IqHandler* ih, int context, bool del = false );

      /**
       * A convenience function that sends the given IQ stanza.
       * @param iq The IQ stanza to send.
       */
      void send( const IQ& iq );

      /**
       * A convenience function that sends the given Message stanza.
       * @param msg The Message stanza to send.
       */
      void send( const Message& msg );

      /**
       * A convenience function that sends the given Subscription stanza.
       * @param sub The Subscription stanza to send.
       */
      void send( const Subscription& sub );

      /**
       * A convenience function that sends the given Presence stanza.
       * @param pres The Presence stanza to send.
       */
      void send( Presence& pres );

      /**
       * Returns whether authentication has taken place and was successful.
       * @return @b True if authentication has been carried out @b and was successful, @b false otherwise.
       */
      bool authed() const { return m_authed; }

      /**
       * Returns the current connection status.
       * @return The status of the connection.
       */
      ConnectionState state() const;

      /**
       * Retrieves the value of the xml:lang attribute of the initial stream.
       * Default is 'en', i.e. if not changed by a call to @ref setXmlLang().
       */
      const std::string& xmlLang() const { return m_xmllang; }

      /**
       * Sets the value for the xml:lang attribute of the initial stream.
       * @param xmllang The language identifier for the stream. It must conform to
       * section 2.12 of the XML specification and RFC 3066.
       * Default is 'en'.
       */
      void setXmlLang( const std::string& xmllang ) { m_xmllang = xmllang; }

      /**
       * This function returns the concrete connection implementation currently in use.
       * @return The concrete connection implementation.
       * @since 0.9
       */
      ConnectionBase* connectionImpl() const { return m_connection; }

      /**
       * Use this function if you have a class implementing a UDP, SCTP (or whatever)
       * connection. This should be called before calling connect(). If there already is a
       * connection implementation set (either manually or automatically), it gets deleted.
       * @param cb The connection to use.
       * @since 0.9
       */
      void setConnectionImpl( ConnectionBase* cb );

      /**
       * This function returns the concrete encryption implementation currently in use.
       * @return The concrete encryption implementation.
       * @since 0.9
       */
      TLSBase* encryptionImpl() const { return m_encryption; }

      /**
       * Use this function if you have a class supporting hardware encryption (or whatever).
       * This should be called before calling connect(). If there already is a
       * encryption implementation set (either manually or automatically), it gets deleted.
       * @param tb The encryption implementation to use.
       * @since 0.9
       */
      void setEncryptionImpl( TLSBase* tb );

      /**
       * This function returns the concrete compression implementation currently in use.
       * @return The concrete compression implementation.
       * @since 0.9
       */
      CompressionBase* compressionImpl() const { return m_compression; }

      /**
       * Use this function if you have a class supporting some fancy compression algorithm.
       * This should be called before calling connect(). If there already is a
       * compression implementation set (either manually or automatically), it gets deleted.
       * @param cb The compression implementation to use.
       * @since 0.9
       */
      void setCompressionImpl( CompressionBase* cb );

      /**
       * Sends a whitespace ping to the server.
       * @since 0.9
       */
      void whitespacePing();

      /**
       * Sends a XMPP Ping (XEP-0199) to the given JID.
       * @param to Then entity to ping.
       * @param eh An EventHandler to inform about the reply.
       * @since 0.9
       */
      void xmppPing( const JID& to, EventHandler* eh );

      /**
       * Use this function to set an authorization ID (authzid). Provided the server supports it
       * and the user has sufficient rights, they could then authenticate as bob@example.net but
       * act as alice@example.net.
       * @param authzid The JID to authorize as. Only the bare JID is used.
       * @since 0.9
       */
      void setAuthzid( const JID& authzid ) { m_authzid = authzid; }

      /**
       * Use this function to set an authentication ID (authcid) for SASL PLAIN.
       * The default authcid is the username, i.e. the JID's node part. This should work in most cases.
       * If this is not what you want to use for authentication, use this function.
       * @param authcid The authentication ID.
       * @since 1.0
       * @note Right now this is used for SASL PLAIN authentication only.
       */
      void setAuthcid( const std::string& authcid ) { m_authcid = authcid; }

      /**
       * Use this function to limit SASL mechanisms gloox can use. By default, all
       * supported mechanisms are allowed. To exclude one (or more) mechanisms, remove
       * it from SaslMechAll like so:
       * @code
       * int mymechs = SaslMechAll ^ SaslMechDigestMd5;
       * @endcode
       * @param mechanisms Bitwise ORed @ref SaslMechanism.
       * @since 0.9
       */
      void setSASLMechanisms( int mechanisms ) { m_availableSaslMechs = mechanisms; }

      /**
       * Registers a new StanzaExtension with the StanzaExtensionFactory.
       * @param ext The extension to register.
       */
      void registerStanzaExtension( StanzaExtension* ext );

      /**
       * Removes the given StanzaExtension type from the StanzaExtensionFactory.
       * @param ext The extension type.
       * @return @b True if the given type was found (and removed), @b false otherwise.
       */
      bool removeStanzaExtension( int ext );

      /**
       * Registers @c cl as object that receives connection notifications.
       * @param cl The object to receive connection notifications.
       */
      void registerConnectionListener( ConnectionListener* cl );

      /**
       * Registers @c ih as object that receives notifications for IQ stanzas
       * that contain StanzaExtensions of the given type. The number of handlers
       * per extension type is not limited.
       * @param ih The object to receive IQ stanza notifications.
       * @param exttype The extension type. See StanzaExtension and
       * @link gloox::StanzaExtensionType StanzaExtensionType @endlink.
       * @since 1.0
       */
      void registerIqHandler( IqHandler* ih, int exttype );

      /**
       * Removes the given IqHandler from the list of handlers of pending operations, added
       * using trackID(). Necessary, for example, when closing a GUI element that has an
       * operation pending.
       * @param ih The IqHandler to remove.
       * @since 0.8.7
       */
      void removeIDHandler( IqHandler* ih );

      /**
       * Registers @c mh as object that receives Message stanza notifications.
       * @param mh The object to receive Message stanza notifications.
       */
      void registerMessageHandler( MessageHandler* mh );

      /**
       * Removes the given object from the list of message handlers.
       * @param mh The object to remove from the list.
       */
      void removeMessageHandler( MessageHandler* mh );

      /**
       * Registers the given MessageSession to receive Messages incoming from the session's
       * target JID.
       * @note The ClientBase instance becomes the owner of the MessageSession, it will be deleted
       * in ClientBase's destructor. To get rid of the session before that, use disposeMessageSession().
       * @param session The MessageSession to register.
       * @note Since a MessageSession automatically registers itself with the ClientBase, there is no
       * need to call this function directly.
       */
      void registerMessageSession( MessageSession* session );

      /**
       * Removes the given MessageSession from the  list of MessageSessions and deletes it.
       * @param session The MessageSession to be deleted.
       */
      void disposeMessageSession( MessageSession* session );

      /**
       * Registers @c ph as object that receives Presence stanza notifications.
       * @param ph The object to receive Presence stanza notifications.
       */
      void registerPresenceHandler( PresenceHandler* ph );

      /**
       * Registers a new PresenceHandler for the given JID. Presences received for this
       * particular JID will not be forwarded to the generic PresenceHandler (and therefore
       * the Roster).
       * This functionality is primarily intended for the MUC implementation.
       * @param jid The JID to 'watch'.
       * @param ph The PresenceHandler to inform about presence changes from @c jid.
       * @since 0.9
       */
      void registerPresenceHandler( const JID& jid, PresenceHandler* ph );

      /**
       * Registers @c sh as object that receives Subscription stanza notifications.
       * @param sh The object to receive Subscription stanza notifications.
       */
      void registerSubscriptionHandler( SubscriptionHandler* sh );

      /**
       * Registers @c th as object that receives incoming packts with a given root tag
       * qualified by the given namespace.
       * @param th The object to receive Subscription packet notifications.
       * @param tag The element's name.
       * @param xmlns The element's namespace.
       */
      void registerTagHandler( TagHandler* th, const std::string& tag,
                                               const std::string& xmlns );

      /**
       * Registers @c sh as object that receives up-to-date connection statistics each time
       * a Stanza is received or sent. Alternatively, you can use getStatistics() manually.
       * Only one StatisticsHandler per ClientBase at a time is possible.
       * @param sh The StatisticsHandler to register.
       */
      void registerStatisticsHandler( StatisticsHandler* sh );

      /**
       * Removes the given object from the list of connection listeners.
       * @param cl The object to remove from the list.
       */
      void removeConnectionListener( ConnectionListener* cl );

      /**
       * Removes the given IQ handler for the given extension type.
       * @param ih The IqHandler.
       * @param exttype The extension type. See
       * @link gloox::StanzaExtensionType StanzaExtensionType @endlink.
       * @since 1.0
       */
      void removeIqHandler( IqHandler* ih, int exttype );

      /**
       * Removes the given object from the list of presence handlers.
       * @param ph The object to remove from the list.
       */
      void removePresenceHandler( PresenceHandler* ph );

      /**
       * Removes the given object from the list of presence handlers for the given JID.
       * @param jid The JID to remove the PresenceHandler(s) for.
       * @param ph The PresenceHandler to remove from the list. If @c ph is 0,
       * all handlers for the given JID will be removed.
       */
      void removePresenceHandler( const JID& jid, PresenceHandler* ph );

      /**
       * Removes the given object from the list of subscription handlers.
       * @param sh The object to remove from the list.
       */
      void removeSubscriptionHandler( SubscriptionHandler* sh );

      /**
       * Removes the given object from the list of tag handlers for the given element and namespace.
       * @param th The object to remove from the list.
       * @param tag The element to remove the handler for.
       * @param xmlns The namespace qualifying the element.
       */
      void removeTagHandler( TagHandler* th, const std::string& tag,
                                             const std::string& xmlns );

      /**
       * Removes the current StatisticsHandler.
       */
      void removeStatisticsHandler();

      /**
       * Use this function to set a number of trusted root CA certificates which shall be
       * used to verify a servers certificate.
       * @param cacerts A list of absolute paths to CA root certificate files in PEM format.
       */
      void setCACerts( const StringList& cacerts ) { m_cacerts = cacerts; }

      /**
       * Use this function to set the user's certificate and private key. The certificate will
       * be presented to the server upon request and can be used for SASL EXTERNAL authentication.
       * The user's certificate file should be a bundle of more than one certificate in PEM format.
       * The first one in the file should be the user's certificate, each cert following that one
       * should have signed the previous one.
       * @note These certificates are not necessarily the same as those used to verify the server's
       * certificate.
       * @param clientKey The absolute path to the user's private key in PEM format.
       * @param clientCerts A path to a certificate bundle in PEM format.
       */
      void setClientCert( const std::string& clientKey, const std::string& clientCerts );

      /**
       * Use this function to register a MessageSessionHandler with the Client.
       * Optionally the MessageSessionHandler can receive only MessageSessions with a given
       * message type. There can be only one handler per message type.<br>
       * A MessageSession will be created for every incoming
       * message stanza if there is no MessageHandler registered for the originating JID.
       * @param msh The MessageSessionHandler that will receive the newly created MessageSession.
       * @param types ORed StanzaSubType's that describe the desired message types the handler
       * shall receive. Only StanzaMessage* types are valid. A value of 0 means any type (default).
       */
      void registerMessageSessionHandler( MessageSessionHandler* msh, int types = 0 );

      /**
       * Returns the LogSink instance for this ClientBase and all related objects.
       * @return The LogSink instance used in the current ClientBase.
       */
      LogSink& logInstance() { return m_logInstance; }

      /**
       * Use this function to retrieve the type of the stream error after it occurs and you received a
       * ConnectionError of type @b ConnStreamError from the ConnectionListener.
       * @return The StreamError.
       * @note The return value is only meaningful when called from ConnectionListener::onDisconnect().
       */
      StreamError streamError() const { return m_streamError; }

      /**
       * Returns the text of a stream error for the given language if available.
       * If the requested language is not available, the default text (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default body
       * will be returned, if any.
       * @return The describing text of a stream error. Empty if no stream error occured.
       */
      const std::string& streamErrorText( const std::string& lang = "default" ) const;

      /**
       * In case the defined-condition element of an stream error contains XML character data you can
       * use this function to retrieve it. RFC 3920 only defines one condition (see-other-host)where
       * this is possible.
       * @return The cdata of the stream error's text element (only for see-other-host).
       */
      const std::string& streamErrorCData() const { return m_streamErrorCData; }

      /**
       * This function can be used to retrieve the application-specific error condition of a stream error.
       * @return The application-specific error element of a stream error. 0 if no respective element was
       * found or no error occured.
       */
      const Tag* streamErrorAppCondition() const { return m_streamErrorAppCondition; }

      /**
       * Use this function to retrieve the type of the authentication error after it occurs and you
       * received a ConnectionError of type @b ConnAuthenticationFailed from the ConnectionListener.
       * @return The type of the authentication, if any, @b AuthErrorUndefined otherwise.
       */
      AuthenticationError authError() const { return m_authError; }

      /**
       * Returns a StatisticsStruct containing byte and stanza counts for the current
       * active connection.
       * @return A struct containing the current connection's statistics.
       */
      StatisticsStruct getStatistics();

      /**
       * Registers a MUCInvitationHandler with the ClientBase.
       * @param mih The MUCInvitationHandler to register.
       */
      void registerMUCInvitationHandler( MUCInvitationHandler* mih );

      /**
       * Removes the currently registered MUCInvitationHandler.
       */
      void removeMUCInvitationHandler();

      /**
       * Adds a StanzaExtension that will be sent with every Presence stanza
       * sent. Capabilities are included by default if you are using a Client.
       * @param se A StanzaExtension to add. If an extension of the same type
       * has been added previously it will be replaced by the new one.
       * Use removePresenceExtension() to remove an extension.
       */
      void addPresenceExtension( StanzaExtension* se );

      /**
       * Removes the StanzaExtension of the given type from the list of Presence
       * StanzaExtensions.
       * Use addPresenceExtension() to replace an already added type.
       */
      bool removePresenceExtension( int type );

      /**
       * Returns the current list of Presence StanzaExtensions.
       * @return The current list of Presence StanzaExtensions.
       */
      const StanzaExtensionList& presenceExtensions() const { return m_presenceExtensions; }

      // reimplemented from ParserHandler
      virtual void handleTag( Tag* tag );

      // reimplemented from CompressionDataHandler
      virtual void handleCompressedData( const std::string& data );

      // reimplemented from CompressionDataHandler
      virtual void handleDecompressedData( const std::string& data );

      // reimplemented from ConnectionDataHandler
      virtual void handleReceivedData( const ConnectionBase* connection, const std::string& data );

      // reimplemented from ConnectionDataHandler
      virtual void handleConnect( const ConnectionBase* connection );

      // reimplemented from ConnectionDataHandler
      virtual void handleDisconnect( const ConnectionBase* connection, ConnectionError reason );

      // reimplemented from TLSHandler
      virtual void handleEncryptedData( const TLSBase* base, const std::string& data );

      // reimplemented from TLSHandler
      virtual void handleDecryptedData( const TLSBase* base, const std::string& data );

      // reimplemented from TLSHandler
      virtual void handleHandshakeResult( const TLSBase* base, bool success, CertInfo &certinfo );

    protected:
      /**
       * This function is called when resource binding yieled an error.
       * @param error A pointer to an Error object that contains more
       * information. May be 0.
       */
      void notifyOnResourceBindError( const Error* error );

      /**
       * This function is called when binding a resource succeeded.
       * @param resource The bound resource.
       */
      void notifyOnResourceBind( const std::string& resource );

      /**
       * This function is called when session creation yieled an error.
       * @param error A pointer to an Error object that contains more
       * information. May be 0.
       */
      void notifyOnSessionCreateError( const Error* error );

      /**
       * This function is called when the TLS handshake completed correctly. The return
       * value is used to determine whether or not the client accepted the server's
       * certificate. If @b false is returned the connection is closed.
       * @param info Information on the server's certificate.
       * @return @b True if the certificate seems trustworthy, @b false otherwise.
       */
      bool notifyOnTLSConnect( const CertInfo& info );

      /**
       * This function is called to notify about successful connection.
       */
      void notifyOnConnect();

      /**
       * This function is used to notify subscribers of stream events.
       * @param event The event to publish.
       */
      void notifyStreamEvent( StreamEvent event );

      /**
       * Disconnects the underlying stream and broadcasts the given reason.
       * @param reason The reason for the disconnect.
       */
      virtual void disconnect( ConnectionError reason );

      /**
       * Sends the stream header.
       */
      void header();

      /**
       * Tells ClientBase that authentication was successful (or not).
       * @param authed Whether or not authentication was successful.
       */
      void setAuthed( bool authed ) { m_authed = authed; }

      /**
       * If authentication failed, this function tells ClientBase
       * the reason.
       * @param e The reason for the authentication failure.
       */
      void setAuthFailure( AuthenticationError e ) { m_authError = e; }

      /**
       * Implementors of this function can check if they support the advertized stream version.
       * The return value indicates whether or not the stream can be handled. A default
       * implementation is provided.
       * @param version The advertized stream version.
       * @return @b True if the stream can be handled, @b false otherwise.
       */
      virtual bool checkStreamVersion( const std::string& version );

      /**
       * Starts authentication using the given SASL mechanism.
       * @param type A SASL mechanism to use for authentication.
       */
      void startSASL( SaslMechanism type );

      /**
       * Releases SASL related resources.
       */
      void processSASLSuccess();

      /**
       * Processes the given SASL challenge and sends a response.
       * @param challenge The SASL challenge to process.
       */
      void processSASLChallenge( const std::string& challenge );

      /**
       * Examines the given Tag for SASL errors.
       * @param tag The Tag to parse.
       */
      void processSASLError( Tag* tag );

      /**
       * Sets the domain to use in SASL NTLM authentication.
       * @param domain The domain.
       */
      void setNTLMDomain( const std::string& domain ) { m_ntlmDomain = domain; }

      /**
       * Starts the TLS handshake.
       */
      void startTls();

      /**
       * Indicates whether or not TLS is supported.
       * @return @b True if TLS is supported, @b false otherwise.
       */
      bool hasTls();

      JID m_jid;                         /**< The 'self' JID. */
      JID m_authzid;                     /**< An optional authorization ID. See setAuthzid(). */
      std::string m_authcid;             /**< An alternative authentication ID. See setAuthcid(). */
      ConnectionBase* m_connection;      /**< The transport connection. */
      TLSBase* m_encryption;             /**< Used for connection encryption. */
      CompressionBase* m_compression;    /**< Used for connection compression. */
      Disco* m_disco;                    /**< The local Service Discovery client. */

      /** A list of permanent presence extensions. */
      StanzaExtensionList m_presenceExtensions;

      std::string m_selectedResource;    /**< The currently selected resource.
                                          * See Client::selectResource() and Client::binRessource(). */
      std::string m_clientCerts;         /**< TLS client certificates. */
      std::string m_clientKey;           /**< TLS client private key. */
      std::string m_namespace;           /**< Default namespace. */
      std::string m_password;            /**< Client's password. */
      std::string m_xmllang;             /**< Default value of the xml:lang attribute. */
      std::string m_server;              /**< The server to connect to, if different from the
                                          * JID's server. */
      std::string m_sid;                 /**< The stream ID. */
      bool m_compressionActive;          /**< Indicates whether or not stream compression
                                          * is currently activated. */
      bool m_encryptionActive;           /**< Indicates whether or not stream encryption
                                          * is currently activated. */
      bool m_compress;                   /**< Whether stream compression
                                          * is desired at all. */
      bool m_authed;                     /**< Whether authentication has been completed successfully. */
      bool m_block;                      /**< Whether blocking connection is wanted. */
      bool m_sasl;                       /**< Whether SASL authentication is wanted. */
      TLSPolicy m_tls;                   /**< The current TLS policy. */
      int m_port;                        /**< The port to connect to, if not to be determined
                                          * by querying the server's SRV records. */

      int m_availableSaslMechs;          /**< The SASL mechanisms the server offered. */

    private:
#ifdef CLIENTBASE_TEST
    public:
#endif
      /**
       * @brief This is an implementation of an XMPP Ping (XEP-199).
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class Ping : public StanzaExtension
      {

        public:
          /**
           * Constructs a new object.
           */
          Ping();

          /**
           * Destructor.
           */
          virtual ~Ping();

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            (void)tag;
            return new Ping();
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const
          {
            return new Tag( "ping", "xmlns", XMLNS_XMPP_PING );
          }

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            return new Ping();
          }

      };

      ClientBase( const ClientBase& );
      ClientBase& operator=( const ClientBase& );

      /**
       * This function is called right after the opening &lt;stream:stream&gt; was received.
       */
      virtual void handleStartNode() = 0;

      /**
       * This function is called for each Tag. Only stream initiation/negotiation should
       * be done here.
       * @param tag A Tag to handle.
       */
      virtual bool handleNormalNode( Tag* tag ) = 0;
      virtual void rosterFilled() = 0;
      virtual void cleanup() {}
      virtual void handleIqIDForward( const IQ& iq, int context ) { (void) iq; (void) context; }

      void parse( const std::string& data );
      void init();
      void handleStreamError( Tag* tag );
      TLSBase* getDefaultEncryption();
      CompressionBase* getDefaultCompression();

      void notifyIqHandlers( IQ& iq );
      void notifyMessageHandlers( Message& msg );
      void notifyPresenceHandlers( Presence& presence );
      void notifySubscriptionHandlers( Subscription& s10n );
      void notifyTagHandlers( Tag* tag );
      void notifyOnDisconnect( ConnectionError e );
      void send( const std::string& xml );
      void addFrom( Tag* tag );
      void addNamespace( Tag* tag );

      // reimplemented from IqHandler
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler
      virtual void handleIqID( const IQ& iq, int context );

      struct TrackStruct
      {
        IqHandler* ih;
        int context;
        bool del;
      };

      struct TagHandlerStruct
      {
        TagHandler* th;
        std::string xmlns;
        std::string tag;
      };

      struct JidPresHandlerStruct
      {
        JID* jid;
        PresenceHandler* ph;
      };

      enum TrackContext
      {
        XMPPPing
      };

      typedef std::list<ConnectionListener*>               ConnectionListenerList;
      typedef std::multimap<const std::string, IqHandler*> IqHandlerMapXmlns;
      typedef std::multimap<const int, IqHandler*>         IqHandlerMap;
      typedef std::map<const std::string, TrackStruct>     IqTrackMap;
      typedef std::map<const std::string, MessageHandler*> MessageHandlerMap;
      typedef std::list<MessageSession*>                   MessageSessionList;
      typedef std::list<MessageHandler*>                   MessageHandlerList;
      typedef std::list<PresenceHandler*>                  PresenceHandlerList;
      typedef std::list<JidPresHandlerStruct>              PresenceJidHandlerList;
      typedef std::list<SubscriptionHandler*>              SubscriptionHandlerList;
      typedef std::list<TagHandlerStruct>                  TagHandlerList;

      ConnectionListenerList   m_connectionListeners;
      IqHandlerMapXmlns        m_iqNSHandlers;
      IqHandlerMap             m_iqExtHandlers;
      IqTrackMap               m_iqIDHandlers;
      MessageSessionList       m_messageSessions;
      MessageHandlerList       m_messageHandlers;
      PresenceHandlerList      m_presenceHandlers;
      PresenceJidHandlerList   m_presenceJidHandlers;
      SubscriptionHandlerList  m_subscriptionHandlers;
      TagHandlerList           m_tagHandlers;
      StringList               m_cacerts;
      StatisticsHandler      * m_statisticsHandler;
      MUCInvitationHandler   * m_mucInvitationHandler;
      MessageSessionHandler  * m_messageSessionHandlerChat;
      MessageSessionHandler  * m_messageSessionHandlerGroupchat;
      MessageSessionHandler  * m_messageSessionHandlerHeadline;
      MessageSessionHandler  * m_messageSessionHandlerNormal;

      util::Mutex m_iqHandlerMapMutex;

      Parser m_parser;
      LogSink m_logInstance;
      StanzaExtensionFactory* m_seFactory;
      EventDispatcher m_dispatcher;

      AuthenticationError m_authError;
      StreamError m_streamError;
      StringMap m_streamErrorText;
      std::string m_streamErrorCData;
      Tag* m_streamErrorAppCondition;

      StatisticsStruct m_stats;

      SaslMechanism m_selectedSaslMech;

      std::string m_ntlmDomain;
      bool m_autoMessageSession;

#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
      CredHandle m_credHandle;
      CtxtHandle m_ctxtHandle;
#endif

  };

}

#endif // CLIENTBASE_H__
