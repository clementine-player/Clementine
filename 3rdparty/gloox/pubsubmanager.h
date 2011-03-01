/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef PUBSUBMANAGER_H__
#define PUBSUBMANAGER_H__

#include "pubsub.h"
#include "dataform.h"
#include "iqhandler.h"
#include "mutex.h"

#include <map>
#include <string>

namespace gloox
{

  class ClientBase;

  namespace PubSub
  {

    class ResultHandler;

    /**
     * @brief This manager is used to interact with PubSub services (XEP-0060).
     *
     * @note PubSub support in gloox is still relatively young and you are most
     * welcome to ask questions, criticize the API and so on.
     *
     * A ResultHandler is used to receive a request's result. Depending on the
     * context, this can be a notification that an item has been succesfully
     * deleted (or not), or the default node configuration for a service.
     *
     * To receive PubSub events:
     * @li Tell ClientBase that you are interested in PubSub events by registering
     * an empty PubSub::Event StanzaExtension
     * @code
     * m_client->registerStanzaExtension( new PubSub::Event() );
     * @endcode
     * @li Implement a MessageHandler and register it with ClientBase, or use the MessageSession interface, at your choice,
     * @li When receiving a Message, check it for a PubSub::Event
     * @code
     * const PubSub::Event* pse = msg.findExtension<PubSub::Event>( ExtPubSubEvent );
     * if( pse )
     * {
     *   // use the Event
     * }
     * else
     * {
     *   // no Event
     * }
     * @endcode
     *
     * To interact with PubSub services, you will need to
     * instantiate a PubSub::Manager and
     * implement the ResultHandler virtual interfaces to be notified of the
     * result of requests.
     *
     * @note A null ResultHandler to a query is not allowed and is a no-op.
     *
     * XEP Version: 1.12
     *
     * @author Jakob Schroeter <js@camaya.net>
     * @author Vincent Thomasset <vthomasset@gmail.com>
     *
     * @since 1.0
     */
    class GLOOX_API Manager : public IqHandler
    {
      public:

        /**
         * Initialize the manager.
         * @param parent Client to which this manager belongs.
         */
        Manager( ClientBase* parent );

        /**
         * Default virtual destructor.
         */
        virtual ~Manager() {}

        /**
         * Subscribe to a node.
         *
         * @param service Service hosting the node.
         * @param node ID of the node to subscribe to.
         * @param handler The ResultHandler.
         * @param jid JID to subscribe. If empty, the client's JID will be used
         *        (ie. self subscription).
         * @param type SubscriptionType of the subscription (Collections only).
         * @param depth Subscription depth. For 'all', use 0 (Collections only).
         * @param expire Subscription expiry. Defaults to the empty string.
         * @return The IQ ID used in the request.
         *
         * @see ResultHandler::handleSubscriptionResult
         */
        const std::string subscribe( const JID& service, const std::string& node,
                                     ResultHandler* handler, const JID& jid = JID(),
                                     SubscriptionObject type = SubscriptionNodes,
                                     int depth = 1, const std::string& expire = EmptyString );

        /**
         * Unsubscribe from a node.
         *
         * @param service Service hosting the node.
         * @param node ID of the node to unsubscribe from.
         * @param subid An optional, additional subscription ID.
         * @param handler ResultHandler receiving the result notification.
         * @param jid JID to unsubscribe. If empty, the client's JID will be
         * used (ie self unsubscription).
         * @return The IQ ID used in the request.
         *
         * @see ResultHandler::handleUnsubscriptionResult
         */
        const std::string unsubscribe( const JID& service,
                                       const std::string& node,
                                       const std::string& subid,
                                       ResultHandler* handler,
                                       const JID& jid = JID() );

        /**
         * Requests the subscription list from a service.
         *
         * @param service Service to query.
         * @param handler The ResultHandler to handle the result.
         * @return The IQ ID used in the request.
         *
         * @see ResultHandler::handleSubscriptions
         */
        const std::string getSubscriptions( const JID& service,
                                            ResultHandler* handler )
        {
          return getSubscriptionsOrAffiliations( service,
                                                 handler,
                                                 GetSubscriptionList );
        }

        /**
         * Requests the affiliation list from a service.
         *
         * @param service Service to query.
         * @param handler The ResultHandler to handle the result.
         * @return The IQ ID used in the request.
         *
         * @see ResultHandler::handleAffiliations
         */
        const std::string getAffiliations( const JID& service,
                                           ResultHandler* handler )
        {
          return getSubscriptionsOrAffiliations( service,
                                                 handler,
                                                 GetAffiliationList );
        }

        /**
         * Requests subscription options.
         *
         * @param service Service to query.
         * @param jid Subscribed entity.
         * @param node Node ID of the node.
         * @param handler The SubscriptionListHandler to handle the result.
         * @return The IQ ID used in the request.
         *
         * @see ResultHandler::handleSubscriptionOptions
         */
        const std::string getSubscriptionOptions( const JID& service,
                                                  const JID& jid,
                                                  const std::string& node,
                                                  ResultHandler* handler)
          { return subscriptionOptions( GetSubscriptionOptions, service, jid, node, handler, 0 ); }

        /**
         * Modifies subscription options.
         *
         * @param service Service to query.
         * @param jid Subscribed entity.
         * @param node Node ID of the node.
         * @param df New configuration. The DataForm will be owned and deleted by the Manager.
         * @param handler The handler to handle the result.
         * @return The IQ ID used in the request.
         *
         * @see ResultHandler::handleSubscriptionOptionsResult
         */
        const std::string setSubscriptionOptions( const JID& service,
                                                  const JID& jid,
                                                  const std::string& node,
                                                  DataForm* df,
                                                  ResultHandler* handler )
          { return subscriptionOptions( SetSubscriptionOptions, service, jid, node, handler, df ); }

        /**
         * Requests the affiliation list for a node.
         *
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param handler The AffiliationListHandler to handle the result.
         *
         * @see ResultHandler::handleAffiliations
         */
        void getAffiliations( const JID& service,
                              const std::string& node,
                              ResultHandler* handler );

        /**
         * Requests items from a node.
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param subid An optional subscription ID.
         * @param maxItems The optional maximum number of items to return.
         * @param handler The handler to handle the result.
         * @return The ID used in the request.
         */
        const std::string requestItems( const JID& service,
                                        const std::string& node,
                                        const std::string& subid,
                                        int maxItems,
                                        ResultHandler* handler);

        /**
         * Requests specific items from a node.
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param subid An optional subscription ID.
         * @param items The list of item IDs to request.
         * @param handler The handler to handle the result.
         * @return The ID used in the request.
         */
        const std::string requestItems( const JID& service,
                                        const std::string& node,
                                        const std::string& subid,
                                        const ItemList& items,
                                        ResultHandler* handler);

        /**
         * Publish an item to a node. The Tag to publish is destroyed
         * by the function before returning.
         *
         * @param service Service hosting the node.
         * @param node ID of the node to delete the item from.
         * @param items One or more items to publish. The items will be owned and deleted by the Manager,
         * even in the error case (empty string returned).
         * @param options An optional DataForm containing publish options. The DataForm will be owned and deleted by the Manager.
         * @param handler The handler to handle the result.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleItemPublication
         */
        const std::string publishItem( const JID& service,
                                       const std::string& node,
                                       ItemList& items,
                                       DataForm* options,
                                       ResultHandler* handler );

        /**
         * Delete an item from a node.
         *
         * @param service Service hosting the node.
         * @param node ID of the node to delete the item from.
         * @param items A list of items to delete (only ID filled in).
         * @param notify Whether or not to notify subscribers about the deletion.
         * @param handler The handler to handle the result.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleItemDeletation
         */
        const std::string deleteItem( const JID& service,
                                      const std::string& node,
                                      const ItemList& items,
                                      bool notify,
                                      ResultHandler* handler );

        /**
         * Creates a new node.
         *
         * @param service Service where to create the new node.
         * @param node The ID of the new node.
         * @param config An optional DataForm that holds the node configuration.
         * The DataForm will be owned and deleted by the Manager.
         * @param handler The handler to handle the result.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleNodeCreation
         */
        const std::string createNode( const JID& service,
                                      const std::string& node,
                                      DataForm* config,
                                      ResultHandler* handler );

        /**
         * Deletes a node.
         *
         * @param service Service where to create the new node.
         * @param node Node ID of the new node.
         * @param handler The handler to handle the result.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleNodeDeletion
         */
        const std::string deleteNode( const JID& service,
                                      const std::string& node,
                                      ResultHandler* handler );

        /**
         * Retrieves the default configuration for a specific NodeType.
         *
         * @param service The queried service.
         * @param type NodeType to get default configuration for.
         * @param handler ResultHandler.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleDefaultNodeConfig
         */
        const std::string getDefaultNodeConfig( const JID& service,
                                                NodeType type,
                                                ResultHandler* handler );

        /**
         * Removes all the items from a node.
         *
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param handler ResultHandler.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleNodePurge
         */
        const std::string purgeNode( const JID& service,
                                     const std::string& node,
                                     ResultHandler* handler );

        /**
         * Requests the subscriber list for a node.
         *
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param handler ResultHandler.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleSubscribers
         */
        const std::string getSubscribers( const JID& service,
                                          const std::string& node,
                                          ResultHandler* handler )
          { return subscriberList( GetSubscriberList, service,
                                   node, SubscriberList(),
                                   handler ); }

        /**
         * Modifies the subscriber list for a node. This function SHOULD only set the
         * subscriber list to those which needs modification.
         *
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param list The subscriber list.
         * @param handler The ResultHandler.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleSubscribers
         */
        const std::string setSubscribers( const JID& service,
                                          const std::string& node,
                                          const SubscriberList& list,
                                          ResultHandler* handler )
          { return subscriberList( SetSubscriberList, service,
                                   node, list, handler ); }

        /**
         * Requests the affiliate list for a node.
         *
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param handler ResultHandler.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleAffiliates
         */
        const std::string getAffiliates( const JID& service,
                                         const std::string& node,
                                         ResultHandler* handler )
          { return affiliateList( GetAffiliateList, service,
                                  node, AffiliateList(),
                                  handler ); }

        /**
         * Modifies the affiliate list for a node.
         *
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param list ResultHandler.
         * @param handler ResultHandler.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleAffiliatesResult
         */
        const std::string setAffiliates( const JID& service,
                                         const std::string& node,
                                         const AffiliateList& list,
                                         ResultHandler* handler )
          { return affiliateList( SetAffiliateList, service,
                                  node, list, handler ); }

        /**
         * Retrieve the configuration (options) of a node.
         *
         * @param service Service hosting the node.
         * @param node ID of the node.
         * @param handler ResultHandler responsible to handle the request result.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleNodeConfig
         */
        const std::string getNodeConfig( const JID& service,
                                         const std::string& node,
                                         ResultHandler* handler )
          { return nodeConfig( service, node, 0, handler ); }

        /**
         * Changes a node's configuration (options).
         *
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param config The node's configuration DataForm.
         * @param handler ResultHandler responsible to handle the request result.
         * @return The ID used in the request.
         *
         * @see ResultHandler::handleNodeConfigResult
         */
        const std::string setNodeConfig( const JID& service,
                                         const std::string& node,
                                         DataForm* config,
                                         ResultHandler* handler  )
          { return nodeConfig( service, node, config, handler ); }

        /**
         * Removes an ID from our tracking lists.
         * @param id The ID to remove.
         * @return @b True if the ID was found and removed, @b false otherwise.
         */
        bool removeID( const std::string& id );

        // reimplemented from DiscoHandler
        void handleDiscoInfoResult( IQ* iq, int context );

        // reimplemented from DiscoHandler
        void handleDiscoItemsResult( IQ* iq, int context );

        // reimplemented from DiscoHandler
        void handleDiscoError( IQ* iq, int context );

        // reimplemented from DiscoHandler
        bool handleDiscoSet( IQ* ) { return 0; }

        // reimplemented from IqHandler.
        virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

        // reimplemented from IqHandler.
        virtual void handleIqID( const IQ& iq, int context );

      private:
#ifdef PUBSUBMANAGER_TEST
      public:
#endif

        enum TrackContext
        {
          Subscription,
          Unsubscription,
          GetSubscriptionOptions,
          SetSubscriptionOptions,
          GetSubscriptionList,
          GetSubscriberList,
          SetSubscriberList,
          GetAffiliationList,
          GetAffiliateList,
          SetAffiliateList,
          GetNodeConfig,
          SetNodeConfig,
          DefaultNodeConfig,
          GetItemList,
          PublishItem,
          DeleteItem,
          CreateNode,
          DeleteNode,
          PurgeNodeItems,
          NodeAssociation,
          NodeDisassociation,
          GetFeatureList,
          DiscoServiceInfos,
          DiscoNodeInfos,
          DiscoNodeItems,
          RequestItems,
          InvalidContext
        };

        class PubSubOwner : public StanzaExtension
        {
          public:
            /**
             * Creates a new PubSubOwner object that can be used to request the given type.
             * @param context The requets type.
             */
            PubSubOwner( TrackContext context = InvalidContext );

            /**
             * Creates a new PubSubOwner object by parsing the given Tag.
             * @param tag The Tag to parse.
             */
            PubSubOwner( const Tag* tag );

            /**
             * Virtual destructor.
             */
            virtual ~PubSubOwner();

            /**
             * Sets the node to use in e.g. subscription requests.
             * @param node The node to use.
             */
            void setNode( const std::string& node ) { m_node = node; }

            /**
             * Returns the pubsub node.
             * @return The pubsub node.
             */
            const std::string& node() const { return m_node; }

            /**
             * Sets an options DataForm.
             * @param options The DataForm.
             */
            void setConfig( DataForm* config )
              { m_form = config; }

            /**
             * Returns the config DataForm.
             * @return The config DataForm.
             */
            const DataForm* config() const { return m_form; }

            /**
             * Sets the subscriber list.
             * @param subList The subscriber list.
             */
            void setSubscriberList( const SubscriberList& subList )
              { m_subList = subList; }

            /**
             * Sets the affiliate list.
             * @param affList The affiliate list.
             */
            void setAffiliateList( const AffiliateList& affList )
              { m_affList = affList; }

            // reimplemented from StanzaExtension
            virtual const std::string& filterString() const;

            // reimplemented from StanzaExtension
            virtual StanzaExtension* newInstance( const Tag* tag ) const
            {
              return new PubSubOwner( tag );
            }

            // reimplemented from StanzaExtension
            virtual Tag* tag() const;

            // reimplemented from StanzaExtension
            virtual StanzaExtension* clone() const
            {
              PubSubOwner* p = new PubSubOwner();
              p->m_node = m_node;
              p->m_ctx = m_ctx;
              p->m_form = m_form ? new DataForm( *m_form ) : 0;
              p->m_subList = m_subList;
              p->m_affList = m_affList;
              return p;
            }

          private:
            std::string m_node;
            TrackContext m_ctx;
            DataForm* m_form;
            SubscriberList m_subList;
            AffiliateList m_affList;
        };

        class PubSub : public StanzaExtension
        {
          public:
            /**
             * Creates a new PubSub object that can be used to request the given type.
             * @param context The requets type.
             */
            PubSub( TrackContext context = InvalidContext );

            /**
             * Creates a new PubSub object by parsing the given Tag.
             * @param tag The Tag to parse.
             */
            PubSub( const Tag* tag );

            /**
             * Virtual destructor.
             */
            virtual ~PubSub();

            /**
             * Sets the JID to use in e.g. subscription requests.
             * @param jid The JID to use.
             */
            void setJID( const JID& jid ) { m_jid = jid; }

            /**
             * Returns the pubsub JID (not the service JID).
             * @return The pubsub JID.
             */
            const JID& jid() const { return m_jid; }

            /**
             * Sets the node to use in e.g. subscription requests.
             * @param node The node to use.
             */
            void setNode( const std::string& node ) { m_node = node; }

            /**
             * Returns the pubsub node.
             * @return The pubsub node.
             */
            const std::string& node() const { return m_node; }

            /**
             * Sets the Subscription ID to use.
             * @param subid The Subscription ID to use.
             */
            void setSubscriptionID( const std::string& subid )
              { m_subid = subid; }

            /**
             * Gets the Subscription ID to use.
             * @return The Subscription ID to use.
             */
            const std::string& subscriptionID() const { return m_subid; }

            /**
             * Sets the subscription options.
             * @param node The node to set the options for.
             * @param df The DataForm holding the subscription options.
             * Will be owned and deleted by the PubSub object
             */
            void setOptions( const std::string& node, DataForm* df )
            {
              m_options.node = node;
              m_options.df = df;
            }

            /**
             * Returns the subscription options.
             * @return The subscription options.
             */
            const DataForm* options() const
              { return m_options.df; }

            /**
             * Returns the current Items.
             * @return The current items.
             */
            const ItemList& items() const { return m_items; }

            /**
             * Sets the subscription IDs.
             * @param ids Subscription IDs.
             */
            void setItems( const ItemList& items )
             { m_items = items; }

            /**
             * Sets the maximum number of items to request.
             * @param maxItems The maximum number of items to request.
             */
            void setMaxItems( int maxItems )
              { m_maxItems = maxItems; }

            /**
             * Returns the subscriptions.
             * @param The subscriptions.
             */
            const SubscriptionMap& subscriptions() const
              { return m_subscriptionMap; }

            /**
             * Returns the affiliations.
             * @param The affiliations.
             */
            const AffiliationMap& affiliations() const
              { return m_affiliationMap; }

            /**
             * Sets whether or not a notify element should be included in a 'retract'.
             * @param notify Indicates whether a notify attribute is included.
             */
            void setNotify( bool notify ) { m_notify = notify; }

            // reimplemented from StanzaExtension
            virtual const std::string& filterString() const;

            // reimplemented from StanzaExtension
            virtual StanzaExtension* newInstance( const Tag* tag ) const
            {
              return new PubSub( tag );
            }

            // reimplemented from StanzaExtension
            virtual Tag* tag() const;

            // reimplemented from StanzaExtension
            virtual StanzaExtension* clone() const;

          private:
            AffiliationMap m_affiliationMap;
            SubscriptionMap m_subscriptionMap;
            TrackContext m_ctx;

            struct Options
            {
              std::string node;
              DataForm* df;
            };
            Options m_options;
            JID m_jid;
            std::string m_node;
            std::string m_subid;
            ItemList m_items;
            int m_maxItems;
            bool m_notify;
        };

        /**
         * This function sets or requests a node's configuration form
         * (depending on arguments). Does the actual work for requestNodeConfig
         * and setNodeConfig.
         * Requests or changes a node's configuration.
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param config If not NULL, the function will request the node config.
         * Otherwise, it will set the config based on the form.
         * @param handler ResultHandler responsible to handle the request result.
         */
        const std::string nodeConfig( const JID& service,
                                      const std::string& node,
                                      DataForm* config,
                                      ResultHandler* handler );

        /**
         * This function sets or requests a node's subscribers list form
         * (depending on arguments). Does the actual work for
         * requestSubscriberList and setSubscriberList.
         * Requests or changes a node's configuration.
         * @param ctx The operation to be performed.
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param config If not NULL, the function will request the node config.
         * Otherwise, it will set the config based on the form.
         * @param handler ResultHandler responsible to handle the request result.
         * @return The ID used in the request.
         */
        const std::string subscriberList( TrackContext ctx,
                                          const JID& service,
                                          const std::string& node,
                                          const SubscriberList& config,
                                          ResultHandler* handler );

        /**
         * This function sets or requests a node's affiliates list
         * (depending on arguments). Does the actual work for
         * requestAffiliateList and setAffiliateList.
         * Requests or changes a node's configuration.
         * @param ctx The operation to be performed.
         * @param service Service to query.
         * @param node Node ID of the node.
         * @param config If not NULL, the function will request the node config.
         * Otherwise, it will set the config based on the form.
         * @param handler ResultHandler responsible to handle the request result.
         * @return The ID used in the request.
         */
        const std::string affiliateList( TrackContext ctx,
                                         const JID& service,
                                         const std::string& node,
                                         const AffiliateList& config,
                                         ResultHandler* handler );

        const std::string subscriptionOptions( TrackContext context,
                                               const JID& service,
                                               const JID& jid,
                                               const std::string& node,
                                               ResultHandler* handler,
                                               DataForm* df );

        const std::string getSubscriptionsOrAffiliations( const JID& service,
            ResultHandler* handler,
            TrackContext context );

        typedef std::map < std::string, std::string > NodeOperationTrackMap;
        typedef std::map < std::string, ResultHandler* > ResultHandlerTrackMap;

        ClientBase* m_parent;

        NodeOperationTrackMap  m_nopTrackMap;
        ResultHandlerTrackMap  m_resultHandlerTrackMap;

        util::Mutex m_trackMapMutex;

    };

  }

}

#endif // PUBSUBMANAGER_H__
