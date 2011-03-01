/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef PUBSUBEVENT_H__
#define PUBSUBEVENT_H__

#include "stanzaextension.h"
#include "pubsub.h"
#include "gloox.h"

namespace gloox
{

  class Tag;

  namespace PubSub
  {

    /**
     * @brief This is an implementation of a PubSub Notification as a StanzaExtension.
     *
     * @author Vincent Thomasset <vthomasset@gmail.com>
     * @since 1.0
     */
    class GLOOX_API Event : public StanzaExtension
    {
      public:

        /**
         * Stores a retract or item notification.
         */
        struct ItemOperation
        {
          /**
           * Constructor.
           *
           * @param remove Whether this is a retract operation or not (ie item).
           * @param itemid Item ID of this item.
           * @param pld Payload for this object (in the case of a non transient
           * item notification).
           */
          ItemOperation( bool remove, const std::string& itemid, const Tag* pld = 0 )
            : retract( remove ), item( itemid ), payload( pld )
            {}

          /**
           * Copy constructor.
           * @param right The ItemOperation to copy from.
           */
          ItemOperation( const ItemOperation& right );

          bool retract;
          std::string item;
          const Tag* payload;
        };

        /**
         * A list of ItemOperations.
         */
        typedef std::list<ItemOperation*> ItemOperationList;

        /**
         * PubSub event notification Stanza Extension.
         * @param event A tag to parse.
         */
        Event( const Tag* event );

        /**
         * PubSub event notification Stanza Extension.
         * @param node The node's ID for which the notification is sent.
         * @param type The event's type.
         */
        Event( const std::string& node, PubSub::EventType type );

        /**
         * Virtual destructor.
         */
        virtual ~Event();

        /**
         * Returns the event's type.
         * @return The event's type.
         */
        PubSub::EventType type() const { return m_type; }

        /**
         * Returns the list of subscription IDs for which this notification
         * is valid.
         * @return The list of subscription IDs.
         */
        const StringList& subscriptions() const
          { return m_subscriptionIDs ? *m_subscriptionIDs : m_emptyStringList; }

        /**
         * Returns the list of ItemOperations for EventItems(Retract) notification.
         * @return The list of ItemOperations.
         */
        const ItemOperationList& items() const
          { return m_itemOperations ? *m_itemOperations : m_emptyOperationList; }

        /**
         * Add an item to the list of ItemOperations for EventItems(Retract) notification.
         * After calling, the PubSub::Event object owns the ItemOperation and will free it.
         * @param op An ItemOperation to add.
         */
        void addItem( ItemOperation* op );

        /**
         * Returns the node's ID for which the notification is sent.
         * @return The node's ID.
         */
        const std::string& node() const { return m_node; }

        /**
         * Returns the subscribe/unsubscribed JID. Only set for subscription notifications
         * (type() == EventSubscription).
         * @return The affected JID.
         */
        const JID& jid() { return m_jid; }

        /**
         * Returns the subscription state. Only set for subscription notifications
         * (type() == EventSubscription).
         * @return @b True if the subscription request was approved, @b false otherwise.
         */
        bool subscription() { return m_subscription; }

        // reimplemented from StanzaExtension
        const std::string& filterString() const;

        // reimplemented from StanzaExtension
        StanzaExtension* newInstance( const Tag* tag ) const
        {
          return new Event( tag );
        }

        // reimplemented from StanzaExtension
        Tag* tag() const;

        // reimplemented from StanzaExtension
        virtual StanzaExtension* clone() const;

      private:
        Event& operator=( const Event& );

        PubSub::EventType m_type;
        std::string m_node;
        StringList* m_subscriptionIDs;
        JID m_jid;
        Tag* m_config;
        ItemOperationList* m_itemOperations;
        std::string m_collection;
        bool m_subscription;

        const ItemOperationList m_emptyOperationList;
        const StringList m_emptyStringList;

    };

  }

}

#endif // PUBSUBEVENT_H__
