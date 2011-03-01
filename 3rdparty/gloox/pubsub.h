/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef PUBSUB_H__
#define PUBSUB_H__

#include <map>
#include <string>

#include "gloox.h"
#include "jid.h"

namespace gloox
{
  /**
   * @brief Namespace holding all the Pubsub-related structures and definitions.
   */
  namespace PubSub
  {

    class Item;

    /**
     * Describes the different node types.
     */
    enum NodeType
    {
      NodeLeaf,                     /**< A node that contains published items only. It is NOT
                                     * a container for other nodes. */
      NodeCollection,               /**< A node that contains nodes and/or other collections but
                                     * no published items. Collections make it possible to represent
                                     * hierarchial node structures. */
      NodeInvalid                   /**< Invalid node type */
    };

    /**
     * Describes the different node affiliation types.
     */
    enum AffiliationType
    {
      AffiliationNone,              /**< No particular affiliation type. */
      AffiliationPublisher,         /**< Entity is allowed to publish items. */
      AffiliationOwner,             /**< Manager for the node. */
      AffiliationOutcast,           /**< Entity is disallowed from subscribing or publishing. */
      AffiliationInvalid            /**< Invalid Affiliation type. */
    };

    /**
     * Describes the different subscription types.
     */
    enum SubscriptionType
    {
      SubscriptionNone,             /**< The node MUST NOT send event notifications or payloads to the
                                     * Entity. */
      SubscriptionSubscribed,       /**< An entity is subscribed to a node. The node MUST send all event
                                     * notifications (and, if configured, payloads) to the entity while it
                                     * is in this state. */
      SubscriptionPending,          /**< An entity has requested to subscribe to a node and the request
                                     * has not yet been approved by a node owner. The node MUST NOT send
                                     * event notifications or payloads to the entity while it is in this
                                     * state. */
      SubscriptionUnconfigured,     /**< An entity has subscribed but its subscription options have not yet
                                     * been configured. The node MAY send event notifications or payloads
                                     * to the entity while it is in this state. The service MAY timeout
                                     * unconfigured subscriptions. */
      SubscriptionInvalid           /**< Invalid subscription type. */
    };

    /**
     * Event types.
     */
    enum EventType
    {
      EventCollection,              /**< A Collection node has been created. */
      EventConfigure,               /**< A node's configuration has changed. */
      EventDelete,                  /**< A node has been deleted. */
      EventItems,                   /**< An item has been created or modified. */
      EventItemsRetract,            /**< An item has been deleted. */
      EventPurge,                   /**< A Leaf node has been purged. */
      EventSubscription,            /**< A user's subscription has been processed. */
      EventUnknown                  /**< Unknown event. */
    };

    /**
     * Describes the different subscription types.
     */
    enum SubscriptionObject
    {
      SubscriptionNodes,            /**< Receive notification of new nodes only. */
      SubscriptionItems             /**< Receive notification of new items only. */
    };

    /**
     * Describes the access types.
     */
    enum AccessModel
    {
      AccessOpen,                   /**< Any entity may subscribe to the node (i.e., without the necessity
                                     * for subscription approval) and any entity may retrieve items from the
                                     * node (i.e., without being subscribed); this SHOULD be the default
                                     * access model for generic pubsub services. */
      AccessPresence,               /**< Any entity with a subscription of type "from" or "both" may subscribe
                                     * to the node and retrieve items from the node; this access model applies
                                     * mainly to instant messaging systems (see RFC 3921). */
      AccessRoster,                 /**< Any entity in the specified roster group(s) may subscribe to the node
                                     * and retrieve items from the node; this access model applies mainly to
                                     * instant messaging systems (see RFC 3921). */
      AccessAuthorize,              /**< The node owner must approve all subscription requests, and only
                                     * subscribers may retrieve items from the node. */
      AccessWhitelist,              /**< An entity may be subscribed only through being added to a whitelist
                                     * by the node owner (unsolicited subscription requests are rejected), and
                                     * only subscribers may retrieve items from the node. In effect, the
                                     * default affiliation is outcast. The node owner MUST automatically be
                                     * on the whitelist. In order to add entities to the whitelist, the
                                     * node owner SHOULD use the protocol specified in the Manage Affiliated
                                     * Entities section of this document. */
      AccessDefault                 /**< Unspecified (default) Access Model (does not represent a real access
                                     * type by itself). */
    };

    /**
     * Describes the different PubSub features (XEP-0060 Sect. 10).
     */
    enum PubSubFeature
    {
      FeatureCollections           = 1,     /**< Collection nodes are supported. RECOMMENDED */
      FeatureConfigNode            = 1<<1,  /**< Configuration of node options is supported. RECOMMENDED */
      FeatureCreateAndConfig       = 1<<2,  /**< Simultaneous creation and configuration of nodes is
                                             * supported. RECOMMENDED */
      FeatureCreateNodes           = 1<<3,  /**< Creation of nodes is supported. RECOMMENDED */
      FeatureDeleteAny             = 1<<4,  /**< Any publisher may delete an item (not only the originating
                                             * publisher). OPTIONAL */
      FeatureDeleteNodes           = 1<<5,  /**< Deletion of nodes is supported. RECOMMENDED */
      FeatureGetPending            = 1<<6,  /**< Retrieval of pending subscription approvals is supported.
                                             * OPTIONAL */
      FeatureInstantNodes          = 1<<7,  /**< Creation of instant nodes is supported. RECOMMENDED */
      FeatureItemIDs               = 1<<8,  /**< Publishers may specify item identifiers. RECOMMENDED */
      FeatureLeasedSubscription    = 1<<9,  /**< Time-based subscriptions are supported. OPTIONAL */
      FeatureManageSubscriptions   = 1<<10, /**< Node owners may manage subscriptions. OPTIONAL */
      FeatureMetaData              = 1<<11, /**< Node meta-data is supported. RECOMMENDED */
      FeatureModifyAffiliations    = 1<<12, /**< Node owners may modify affiliations. OPTIONAL */
      FeatureMultiCollection       = 1<<13, /**< A single leaf node may be associated with multiple
                                             * collections. OPTIONAL */
      FeatureMultiSubscribe        = 1<<14, /**< A single entity may subscribe to a node multiple times.
                                             * OPTIONAL */
      FeaturePutcastAffiliation    = 1<<15, /**< The outcast affiliation is supported. RECOMMENDED */
      FeaturePersistentItems       = 1<<16, /**< Persistent items are supported. RECOMMENDED */
      FeaturePresenceNotifications = 1<<17, /**< Presence-based delivery of event notifications is supported.
                                             * OPTIONAL */
      FeaturePublish               = 1<<18, /**< Publishing items is supported (note: not valid for collection
                                             * nodes). REQUIRED */
      FeaturePublisherAffiliation  = 1<<19, /**< The publisher affiliation is supported. OPTIONAL */
      FeaturePurgeNodes            = 1<<20, /**< Purging of nodes is supported. OPTIONAL */
      FeatureRetractItems          = 1<<21, /**< Item retraction is supported. OPTIONAL */
      FeatureRetrieveAffiliations  = 1<<22, /**< Retrieval of current affiliations is supported.
                                             * RECOMMENDED */
      FeatureRetrieveDefault       = 1<<23, /**< Retrieval of default node configuration is supported.
                                             * RECOMMENDED */
      FeatureRetrieveItems         = 1<<24, /**< Item retrieval is supported. RECOMMENDED */
      FeatureRetrieveSubscriptions = 1<<25, /**< Retrieval of current subscriptions is supported.
                                             * RECOMMENDED */
      FeatureSubscribe             = 1<<26, /**< Subscribing and unsubscribing are supported. REQUIRED */
      FeatureSubscriptionOptions   = 1<<27, /**< Configuration of subscription options is supported.
                                             * OPTIONAL */
      FeatureSubscriptionNotifs    = 1<<28, /**< Notification of subscription state changes is supported. */
      FeatureUnknown               = 1<<29  /**< Unrecognized feature */
    };

// [Persistent - Notification]
/* Publisher MUST include an &lt;item/&gt; element, which MAY be empty or contain a payload; if item ID is not provided by publisher, it MUST be generated by pubsub service */

// [Persistent - Payload]
/* Publisher MUST include an &lt;item/&gt; element that contains the payload; if item ID is not provided by publisher, it MUST be generated by pubsub service */

// [Transient - Notification]
/* Publisher MUST NOT include an &lt;item/&gt; element (therefore item ID is neither provided nor generated) but the notification will include an empty &lt;items/&gt; element */

// [Transient - Payload]
/* Publisher MUST include an &lt;item/&gt; element that contains the payload, but the item ID is OPTIONAL */

    /**
     * Describes a subscribed entity.
     */
    struct Subscriber
    {
      Subscriber( const JID& _jid,
                  SubscriptionType _type,
                  const std::string& _subid = EmptyString)
        : jid( _jid ), type( _type ), subid( _subid ) {}
      JID jid;
      SubscriptionType type;
      std::string subid;
    };

    /**
     * Describes an Affiliate.
     */
    struct Affiliate
    {
      Affiliate( const JID& _jid, AffiliationType _type )
        : jid( _jid ), type( _type ) {}
      JID jid;
      AffiliationType type;
    };

    typedef std::list<Subscriber> SubscriberList;
    typedef std::list<Affiliate> AffiliateList;

    /**
     * Struct used to track info between requests.
     *
     */
    struct TrackedInfo
    {
      JID service;
      std::string node;
      std::string item;
      std::string sid;
    };

    /**
     * Struct used for subscription info.
     */
    struct SubscriptionInfo
    {
      SubscriptionType type;
      JID jid;
      std::string subid;
    };

    typedef std::list<SubscriptionInfo> SubscriptionList;
    typedef std::map<std::string, SubscriptionList> SubscriptionMap;
    typedef std::map<std::string, AffiliationType> AffiliationMap;
    typedef std::list<Item*> ItemList;

  }

}

#endif // PUBSUB_H__
