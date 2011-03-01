/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef DISCO_H__
#define DISCO_H__

#include "gloox.h"

#include "iqhandler.h"
#include "jid.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class ClientBase;
  class DataForm;
  class DiscoHandler;
  class DiscoNodeHandler;
  class IQ;

  /**
   * @brief This class implements XEP-0030 (Service Discovery) and XEP-0092 (Software Version).
   *
   * ClientBase will automatically instantiate a Disco object. It can be used to
   * announce special features of your client, or its version, or...
   *
   * XEP version: 2.2
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API Disco : public IqHandler
  {
    friend class ClientBase;

    public:

      class Identity; // declared below class Info

      /**
       * A list of pointers to Identity objects. Used with Disco::Info.
       */
      typedef std::list<Identity*> IdentityList;

      /**
       * @brief An abstraction of a Disco Info element (from Service Discovery, XEP-0030)
       * as a StanzaExtension.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class GLOOX_API Info : public StanzaExtension
      {
        friend class Disco;

        public:
          /**
           * Returns the queried node identifier, if any.
           * @return The node identifier. May be empty.
           */
          const std::string& node() const { return m_node; }

          /**
           * Returns the entity's supported features.
           * @return A list of supported features/namespaces.
           */
          const StringList& features() const { return m_features; }

          /**
           * Use this function to check if the entity the Info came from supports agiven feature.
           * @param feature The feature to check for.
           * @return @b True if the entity announces support for the feature, @b false otherwise.
           */
          bool hasFeature( const std::string& feature ) const;

          /**
           * Returns the entity's identities.
           * @return A list of pointers to Identity objects.
           */
          const IdentityList& identities() const { return m_identities; }

          /**
           * Returns an optionally included data form. This is used by e.g. MUC (XEP-0045).
           * @return An optional data form included in the disco#info. May be 0.
           */
          const DataForm* form() const { return m_form; }

          /**
           * Adds an optional DataForm, e.g. for XEP-0232. Only one form can be added
           * at this point.
           * @param form An optional DataForm to include in the Info reply.
           * The form will be owned by and deleted on destruction of the Info object.
           * @note If called more than once the previously set form will be deleted.
           */
          void setForm( DataForm* form );

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Info( tag );
          }

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            return new Info( *this );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

        private:
#ifdef DISCO_INFO_TEST
        public:
#endif
          /**
           * Creates a empty Info object, suitable for making disco#info requests.
           * @param node The node identifier to query (optional).
           * @param defaultFeatures Indicates whether or not the default features should be
           * included in the Info. Should be @b false for requests, @b true for replies.
           * Defaults to @b false.
           */
          Info( const std::string& node = EmptyString, bool defaultFeatures = false );

           /**
           * Creates an Info object from the given Tag.
           * @param tag A &lt;query&gt; tag in the disco#info namespace, (possibly) containing
           * a disco#info reply.
           */
          Info( const Tag* tag );

          /**
           * Copy constructor.
           * @param info An Info object to copy.
           */
          Info( const Info& info );

          /**
           * Virtual destructor.
           */
          virtual ~Info();

          /**
           * Sets the current info node.
           * @param node The info node.
           */
          void setNode( const std::string& node ) { m_node = node; }

          /**
           * This function can be used to set the entity's features.
           * @param features A list of supported features/namespaces.
           */
          void setFeatures( const StringList& features )
          {
            StringList fl( features );
            fl.sort(); // needed on win32
            m_features.merge( fl );
          }

          /**
           * This function can be used to set the entity's identities.
           * @param identities A list of pointers to the entity's identities.
           * @note The Identity objects pointed to will be owned by the Info object. The
           * list should neither be used again nor should the Identity objects be deleted.
           */
          void setIdentities( const IdentityList& identities ) { m_identities = identities; }

          std::string m_node;
          StringList m_features;
          IdentityList m_identities;
          DataForm* m_form;
      };

      /**
       * @brief An abstraction of a Disco identity (Service Discovery, XEP-0030).
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class GLOOX_API Identity
      {
        friend class Info;
        friend class Disco;

        public:
          /**
           * Constructs a Disco Identity from a category, type and name.
           * See http://www.xmpp.org/registrar/disco-categories.html for more info.
           * @param category The identity's category.
           * @param type The identity's type.
           * @param name The identity's name.
           */
          Identity( const std::string& category,
                    const std::string& type,
                    const std::string& name );

          /**
           * Copy Contructor.
           * @param id An Identity to create a new Identity object from.
           */
          Identity( const Identity& id );

          /**
           * Destructor.
           */
          ~Identity();

          /**
           * Returns the identity's category.
           * @return The identity's category.
           */
          const std::string& category() const { return m_category; }

          /**
           * Returns the identity's type.
           * @return The identity's type.
           */
          const std::string& type() const { return m_type; }

          /**
           * Returns the identity's name.
           * @return The identity's name.
           */
          const std::string& name() const { return m_name; }

          /**
           * Creates and returns a Tag representation of this identity.
           * @return A Tag, or 0.
           */
          Tag* tag() const;

        private:
          /**
           * Creates a Disco Identity from the given Tag.
           * @param tag A Tag representation of a disco identity.
           */
          Identity( const Tag* tag );

          std::string m_category;   /**< The identity's category. */
          std::string m_type;       /**< The identity's type. */
          std::string m_name;       /**< The identity's name. */

      };

      class Item; // declared below class Items

      /**
       * A list of pointers to Item objects. Used with Disco::Items.
       */
      typedef std::list<Item*> ItemList;

      /**
       * @brief An abstraction of a Disco query element (from Service Discovery, XEP-0030)
       * in the disco#items namespace, implemented as a StanzaExtension.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class GLOOX_API Items : public StanzaExtension
      {
        friend class Disco;

        public:
          // This needs to be public so one can proactively send a list of adhoc commands
          // see XEP-0050
          /**
           * Creates an empty Items object, suitable for making disco#items requests.
           * @param node The node identifier to query (optional).
           */
          Items( const std::string& node = EmptyString );

          /**
           * Virtual destructor.
           */
          virtual ~Items();

          /**
           * This function can be used to set the entity's/node's items.
           * @param items A list of pointers to the entity's/node's items.
           * @note The Item objects pointed to will be owned by the Items object. The
           * list should neither be used again nor should the Item objects be deleted.
           */
          void setItems( const ItemList& items );

          /**
           * Returns the queried node identifier, if any.
           * @return The node identifier. May be empty.
           */
          const std::string& node() const { return m_node; }

          /**
           * Returns the entity's/node's items.
           * @return A list of pointers to Item objects.
           */
          const ItemList& items() const { return m_items; }

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Items( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            return new Items( *this );
          }

        private:
#ifdef DISCO_ITEMS_TEST
        public:
#endif
          /**
           * Creates an Items object from the given Tag.
           * @param tag A &lt;query&gt; tag in the disco#items namespace, (possibly) containing
           * a disco#items reply.
           */
          Items( const Tag* tag );

          std::string m_node;
          ItemList m_items;
      };

      /**
       * @brief An abstraction of a Disco item (Service Discovery, XEP-0030).
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class GLOOX_API Item
      {
        friend class Items;

        public:
          /**
           * Constructs a Disco Item from a JID, node and name.
           * @param jid The item's JID.
           * @param node The item's type.
           * @param name The item's name.
           */
          Item( const JID& jid,
                const std::string& node,
                const std::string& name )
          : m_jid( jid ), m_node( node ), m_name( name ) {}

          /**
           * Destructor.
           */
          ~Item() {}

          /**
           * Returns the item's JID.
           * @return The item's JID.
           */
          const JID& jid() const { return m_jid; }

          /**
           * Returns the item's node.
           * @return The item's node.
           */
          const std::string& node() const { return m_node; }

          /**
           * Returns the item's name.
           * @return The item's name.
           */
          const std::string& name() const { return m_name; }

          /**
           * Creates and returns a Tag representation of this item.
           * @return A Tag, or 0.
           */
          Tag* tag() const;

        private:
          /**
           * Creates a Disco Item from the given Tag.
           * @param tag A Tag representation of a Disco item.
           */
          Item( const Tag* tag );

          JID m_jid;                /**< The item's jid. */
          std::string m_node;       /**< The item's type. */
          std::string m_name;       /**< The item's name. */

      };

      /**
       * Adds a feature to the list of supported Jabber features.
       * The list will be posted as an answer to IQ queries in the
       * "http://jabber.org/protocol/disco#info" namespace.
       * These IQ packets will also be forwarded to the
       * application's IqHandler, if it listens to the @c disco\#info namespace.
       * By default, disco(very) queries are handled by the library.
       * By default, all supported, not disabled features are announced.
       * @param feature A feature (namespace) the host app supports.
       * @note Use this function for non-queryable features. For nodes that shall
       * answer to @c disco\#info queries, use registerNodeHandler().
       */
      void addFeature( const std::string& feature )
        { m_features.push_back( feature ); }

      /**
       * Removes the given feature from the list of advertised client features.
       * @param feature The feature to remove.
       * @since 0.9
       */
      void removeFeature( const std::string& feature )
        { m_features.remove( feature ); }

      /**
       * Lets you retrieve the features this Disco instance supports.
       * @param defaultFeatures Include default features. Defaults to @b false.
       * @return A list of supported features/namespaces.
       */
      const StringList features( bool defaultFeatures = false ) const;

      /**
       * Queries the given JID for general infomation according to
       * XEP-0030 (Service Discovery).
       * To receive the results inherit from DiscoHandler and register with the Disco object.
       * @param to The destination-JID of the query.
       * @param node An optional node to query. Not inserted if empty.
       * @param dh The DiscoHandler to notify about results.
       * @param context A context identifier.
       * @param tid An optional id that is going to be used as the IQ request's id. Only
       * necessary if you need to know the request's id.
       */
      void getDiscoInfo( const JID& to, const std::string& node, DiscoHandler* dh, int context,
                         const std::string& tid = EmptyString )
        { getDisco( to, node, dh, context, GetDiscoInfo, tid ); }

      /**
       * Queries the given JID for its items according to
       * XEP-0030 (Service Discovery).
       * To receive the results inherit from DiscoHandler and register with the Disco object.
       * @param to The destination-JID of the query.
       * @param node An optional node to query. Not inserted if empty.
       * @param dh The DiscoHandler to notify about results.
       * @param context A context identifier.
       * @param tid An optional id that is going to be used as the IQ request's id. Only
       * necessary if you need to know the request's id.
       */
      void getDiscoItems( const JID& to, const std::string& node, DiscoHandler* dh, int context,
                          const std::string& tid = EmptyString )
        { getDisco( to, node, dh, context, GetDiscoItems, tid ); }

      /**
       * Sets the version of the host application using this library.
       * The library takes care of jabber:iq:version requests. These
       * IQ packets will not be forwarded to the IqHandlers.
       * @param name The name to be returned to inquireing clients.
       * @param version The version to be returned to inquireing clients.
       * @param os The operating system to announce. Default: don't include.
       */
      void setVersion( const std::string& name, const std::string& version,
                       const std::string& os = EmptyString );

      /**
       * Returns the application's advertised name.
       * @return The application's advertised name.
       */
      const std::string& name() const { return m_versionName; }

      /**
       * Returns the application's advertised version.
       * @return The application's advertised version.
       */
      const std::string& version() const { return m_versionVersion; }

      /**
       * Returns the application's advertised operating system.
       * @return The application's advertised operating system.
       */
      const std::string& os() const { return m_versionOs; }

      /**
       * Sets the identity of this entity.
       * The library uses this information to answer disco#info requests
       * with a correct identity.
       * XEP-0030 requires an entity to have at least one identity. See XEP-0030
       * for more information on categories and types.
       * @param category The entity category of this client. Default: client.
       * @param type The type of this entity. Default: bot.
       * @param name The name of the entity. Default: empty.
       * @note An entity can have more than one identity. You cann add more identities
       * using addIdentity(). A call to setIdentity() will clear the list of identities
       * and, after that, add the new identity given by the arguments to setIdentity().
       */
      void setIdentity( const std::string& category, const std::string& type,
                        const std::string& name = EmptyString );

      /**
       * Adds another identity to the list of identities.
       * @param category The entity category of this client. Default: client.
       * @param type The type of this entity. Default: bot.
       * @param name The name of the entity. Default: empty.
       */
      void addIdentity( const std::string& category, const std::string& type,
                        const std::string& name = EmptyString )
        { m_identities.push_back( new Identity( category, type, name ) ); }

      /**
       * Returns the entity's identities.
       * @return The entity's identities.
       */
      const IdentityList& identities() const { return m_identities; }

      /**
       * Adds an optional DataForm to Disco:Info replies, e.g. for XEP-0232.
       * Only one form can be added at this point.
       * @param form An optional DataForm to include in the Info reply.
       * The form will be owned by and deleted on destruction of the Disco object.
       * @note If called more than once the previously set form will be deleted.
       */
      void setForm( DataForm* form );

      /**
       * Returns the DataForm set by setForm(). Used by Capabilities.
       * @return The DataForm, or 0.
       */
      const DataForm* form() const { return m_form; }

      /**
       * Use this function to register an @ref DiscoHandler with the Disco
       * object. This is only necessary if you want to receive Disco-set requests. Else
       * a one-time registration happens when calling getDiscoInfo() and getDiscoItems(), respectively.
       * @param dh The DiscoHandler-derived object to register.
       */
      void registerDiscoHandler( DiscoHandler* dh )
        { m_discoHandlers.push_back( dh ); }

      /**
       * Unregisters the given DiscoHandler.
       * @param dh The DiscoHandler to unregister.
       */
      void removeDiscoHandler( DiscoHandler* dh );

      /**
       * Use this function to register a @ref DiscoNodeHandler with the Disco
       * object. The DiscoNodeHandler will receive disco#items queries which are
       * directed to the corresponding node registered for the handler.
       * @param nh The NodeHandler-derived object to register.
       * @param node The node name to associate with this handler. Use an empty string to
       * register for the root node.
       */
      void registerNodeHandler( DiscoNodeHandler* nh, const std::string& node );

      /**
       * Removes the node handler for the given node.
       * @param nh The NodeHandler to unregister.
       * @param node The node for which the handler shall be removed. Use an empty string to
       * remove the root node's handler.
       */
      void removeNodeHandler( DiscoNodeHandler* nh, const std::string& node );

      /**
       * Removes all registered nodes of the given node handler.
       * @param nh The NodeHandler to unregister.
       */
      void removeNodeHandlers( DiscoNodeHandler* nh );

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:
#ifdef DISCO_TEST
    public:
#endif
      Disco( ClientBase* parent );
      virtual ~Disco();

      enum IdType
      {
        GetDiscoInfo,
        GetDiscoItems
      };

      void getDisco( const JID& to, const std::string& node, DiscoHandler* dh,
                     int context, IdType idType, const std::string& tid );

      struct DiscoHandlerContext
      {
        DiscoHandler* dh;
        int context;
      };

      ClientBase* m_parent;

      typedef std::list<DiscoHandler*> DiscoHandlerList;
      typedef std::list<DiscoNodeHandler*> DiscoNodeHandlerList;
      typedef std::map<std::string, DiscoNodeHandlerList> DiscoNodeHandlerMap;
      typedef std::map<std::string, DiscoHandlerContext> DiscoHandlerMap;

      DiscoHandlerList m_discoHandlers;
      DiscoNodeHandlerMap m_nodeHandlers;
      DiscoHandlerMap m_track;
      IdentityList m_identities;
      StringList m_features;
      StringMap m_queryIDs;
      DataForm* m_form;

      std::string m_versionName;
      std::string m_versionVersion;
      std::string m_versionOs;

  };

}

#endif // DISCO_H__
