/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef PUBSUBRESULTHANDLER_H__
#define PUBSUBRESULTHANDLER_H__

#include "error.h"
#include "jid.h"
#include "macros.h"
#include "pubsub.h"
#include "tag.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class Tag;
  class Error;
  class DataForm;

  namespace PubSub
  {
    /**
     * @brief A virtual interface to receive item related requests results.
     *
     * Derive from this interface and pass it to item related requests.
     *
     * As a general rule, methods receive an Error pointer which will be null
     * (when the request was successful) or describe the problem. Requests
     * asking for information will have their "pointer to information" set to
     * null when an error occured (that is they're mutually exclusive). In both
     * cases, gloox takes care of deleting these objects.
     *
     * @author Vincent Thomasset <vthomasset@gmail.com>
     * @since 1.0
     */
    class GLOOX_API ResultHandler
    {
      public:
        /**
         * Virtual destructor.
         */
        virtual ~ResultHandler() {}

        /**
         * Receives the payload for an item.
         *
         * @param service Service hosting the queried node.
         * @param node ID of the parent node.
         * @param entry The complete item Tag (do not delete).
         */
        virtual void handleItem( const JID& service,
                                 const std::string& node,
                                 const Tag* entry ) = 0;

        /**
         * Receives the list of Items for a node.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the queried node.
         * @param node ID of the queried node (empty for the root node).
         * @param itemList List of contained items.
         * @param error Describes the error case if the request failed.
         *
         * @see Manager::requestItems()
         */
        virtual void handleItems( const std::string& id,
                                  const JID& service,
                                  const std::string& node,
                                  const ItemList& itemList,
                                  const Error* error = 0 ) = 0;

        /**
         * Receives the result for an item publication.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the queried node.
         * @param node ID of the queried node. If empty, the root node has been queried.
         * @param itemList List of contained items.
         * @param error Describes the error case if the request failed.
         *
         * @see Manager::publishItem
         */
        virtual void handleItemPublication( const std::string& id,
                                            const JID& service,
                                            const std::string& node,
                                            const ItemList& itemList,
                                            const Error* error = 0 ) = 0;

        /**
         * Receives the result of an item removal.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the queried node.
         * @param node ID of the queried node. If empty, the root node has been queried.
         * @param itemList List of contained items.
         * @param error Describes the error case if the request failed.
         *
         * @see Manager::deleteItem
         */
        virtual void handleItemDeletion( const std::string& id,
                                         const JID& service,
                                         const std::string& node,
                                         const ItemList& itemList,
                                         const Error* error = 0 ) = 0;

        /**
         * Receives the subscription results. In case a problem occured, the
         * Subscription ID and SubscriptionType becomes irrelevant.
         *
         * @param id The reply IQ's id.
         * @param service PubSub service asked for subscription.
         * @param node Node asked for subscription.
         * @param sid Subscription ID.
         * @param jid Subscribed entity.
         * @param subType Type of the subscription.
         * @param error Subscription Error.
         *
         * @see Manager::subscribe
         */
        virtual void handleSubscriptionResult( const std::string& id,
                                               const JID& service,
                                               const std::string& node,
                                               const std::string& sid,
                                               const JID& jid,
                                               const SubscriptionType subType,
                                               const Error* error = 0 ) = 0;

        /**
         * Receives the unsubscription results. In case a problem occured, the
         * subscription ID becomes irrelevant.
         *
         * @param id The reply IQ's id.
         * @param service PubSub service.
         * @param error Unsubscription Error.
         *
         * @see Manager::unsubscribe
         */
        virtual void handleUnsubscriptionResult( const std::string& id,
                                                 const JID& service,
                                                 const Error* error = 0 ) = 0;

        /**
         * Receives the subscription options for a node.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the queried node.
         * @param jid Subscribed entity.
         * @param node ID of the node.
         * @param options Options DataForm.
         * @param error Subscription options retrieval Error.
         *
         * @see Manager::getSubscriptionOptions
         */
        virtual void handleSubscriptionOptions( const std::string& id,
                                                const JID& service,
                                                const JID& jid,
                                                const std::string& node,
                                                const DataForm* options,
                                                const Error* error = 0 ) = 0;

        /**
         * Receives the result for a subscription options modification.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the queried node.
         * @param jid Subscribed entity.
         * @param node ID of the queried node.
         * @param error Subscription options modification Error.
         *
         * @see Manager::setSubscriptionOptions
         */
        virtual void handleSubscriptionOptionsResult( const std::string& id,
                                                      const JID& service,
                                                      const JID& jid,
                                                      const std::string& node,
                                                      const Error* error = 0 ) = 0;


        /**
         * Receives the list of subscribers to a node.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the queried node.
         * @param list Subscriber list.
         * @param error Subscription options modification Error.
         *
         * @see Manager::getSubscribers
         */
        virtual void handleSubscribers( const std::string& id,
                                        const JID& service,
                                        const std::string& node,
                                        const SubscriberList* list,
                                        const Error* error = 0 ) = 0;

        /**
         * Receives the result of a subscriber list modification.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the queried node.
         * @param list Subscriber list.
         * @param error Subscriber list modification Error.
         *
         * @see Manager::setSubscribers
         */
        virtual void handleSubscribersResult( const std::string& id,
                                              const JID& service,
                                              const std::string& node,
                                              const SubscriberList* list,
                                              const Error* error = 0 ) = 0;

        /**
         * Receives the affiliate list for a node.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the queried node.
         * @param list Affiliation list.
         * @param error Affiliation list retrieval Error.
         *
         * @see Manager::getAffiliations
         */
        virtual void handleAffiliates( const std::string& id,
                                       const JID& service,
                                       const std::string& node,
                                       const AffiliateList* list,
                                       const Error* error = 0 ) = 0;

        /**
         * Handle the affiliate list for a specific node.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the node.
         * @param list The Affiliate list.
         * @param error Affiliation list modification Error.
         *
         * @see Manager::setAffiliations
         */
        virtual void handleAffiliatesResult( const std::string& id,
                                             const JID& service,
                                             const std::string& node,
                                             const AffiliateList* list,
                                             const Error* error = 0 ) = 0;


        /**
         * Receives the configuration for a specific node.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the node.
         * @param config Configuration DataForm.
         * @param error Configuration retrieval Error.
         *
         * @see Manager::getNodeConfig
         */
        virtual void handleNodeConfig( const std::string& id,
                                       const JID& service,
                                       const std::string& node,
                                       const DataForm* config,
                                       const Error* error = 0 ) = 0;

        /**
         * Receives the result of a node's configuration modification.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the node.
         * @param error Configuration modification Error.
         *
         * @see Manager::setNodeConfig
         */
        virtual void handleNodeConfigResult( const std::string& id,
                                             const JID& service,
                                             const std::string& node,
                                             const Error* error = 0 ) = 0;

        /**
         * Receives the result of a node creation.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the node.
         * @param error Node creation Error.
         *
         * @see Manager::setNodeConfig
         */
        virtual void handleNodeCreation( const std::string& id,
                                         const JID& service,
                                         const std::string& node,
                                         const Error* error = 0 ) = 0;

        /**
         * Receives the result for a node removal.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the node.
         * @param error Node removal Error.
         *
         * @see Manager::deleteNode
         */
        virtual void handleNodeDeletion( const std::string& id,
                                         const JID& service,
                                         const std::string& node,
                                         const Error* error = 0 ) = 0;


        /**
         * Receives the result of a node purge request.
         *
         * @param id The reply IQ's id.
         * @param service Service hosting the node.
         * @param node ID of the node.
         * @param error Node purge Error.
         *
         * @see Manager::purgeNode
         */
        virtual void handleNodePurge( const std::string& id,
                                      const JID& service,
                                      const std::string& node,
                                      const Error* error = 0 ) = 0;

        /**
         * Receives the Subscription list for a specific service.
         *
         * @param id The reply IQ's id.
         * @param service The queried service.
         * @param subMap The map of node's subscription.
         * @param error Subscription list retrieval Error.
         *
         * @see Manager::getSubscriptions
         */
        virtual void handleSubscriptions( const std::string& id,
                                          const JID& service,
                                          const SubscriptionMap& subMap,
                                          const Error* error = 0) = 0;

        /**
         * Receives the Affiliation map for a specific service.
         *
         * @param id The reply IQ's id.
         * @param service The queried service.
         * @param affMap The map of node's affiliation.
         * @param error Affiliation list retrieval Error.
         *
         * @see Manager::getAffiliations
         */
        virtual void handleAffiliations( const std::string& id,
                                         const JID& service,
                                         const AffiliationMap& affMap,
                                         const Error* error = 0 ) = 0;

        /**
         * Receives the default configuration for a specific node type.
         *
         * @param id The reply IQ's id.
         * @param service The queried service.
         * @param config Configuration form for the node type.
         * @param error Default node config retrieval Error.
         *
         * @see Manager::getDefaultNodeConfig
         */
        virtual void handleDefaultNodeConfig( const std::string& id,
                                              const JID& service,
                                              const DataForm* config,
                                              const Error* error = 0 ) = 0;

    };

  }

}

#endif // PUBSUBRESULTHANDLER_H__

