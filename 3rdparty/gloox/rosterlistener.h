/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef ROSTERLISTENER_H__
#define ROSTERLISTENER_H__

#include "rosteritem.h"

#include <string>
#include <map>

namespace gloox
{

  class IQ;
  class Presence;

  /**
   * A map of JID/RosterItem pairs.
   */
  typedef std::map<const std::string, RosterItem*> Roster;

  /**
   * @brief A virtual interface which can be reimplemented to receive roster updates.
   *
   * A class implementing this interface and being registered as RosterListener with the
   * RosterManager object receives notifications about all the changes in the server-side
   * roster. Only one RosterListener per Roster at a time is possible.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API RosterListener
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~RosterListener() {}

      /**
       * Reimplement this function if you want to be notified about new items
       * on the server-side roster (items subject to a so-called Roster Push).
       * This function will be called regardless who added the item, either this
       * resource or another. However, it will not be called for JIDs for which
       * presence is received without them being on the roster.
       * @param jid The new item's full address.
       */
      virtual void handleItemAdded( const JID& jid ) = 0;

      /**
       * Reimplement this function if you want to be notified about items
       * which authorised subscription.
       * @param jid The authorising item's full address.
       */
      virtual void handleItemSubscribed( const JID& jid ) = 0;

      /**
       * Reimplement this function if you want to be notified about items that
       * were removed from the server-side roster (items subject to a so-called Roster Push).
       * This function will be called regardless who deleted the item, either this resource or
       * another.
       * @param jid The removed item's full address.
       */
      virtual void handleItemRemoved( const JID& jid ) = 0;

      /**
       * Reimplement this function if you want to be notified about items that
       * were modified on the server-side roster (items subject to a so-called Roster Push).
       * A roster push is initiated if a second resource of this JID modifies an item stored on the
       * server-side contact list. This can include modifying the item's name, its groups, or the
       * subscription status. These changes are pushed by the server to @b all connected resources.
       * This is why this function will be called if you modify a roster item locally and synchronize
       * it with the server.
       * @param jid The modified item's full address.
       */
      virtual void handleItemUpdated( const JID& jid ) = 0;

      /**
       * Reimplement this function if you want to be notified about items which
       * removed subscription authorization.
       * @param jid The item's full address.
       */
      virtual void handleItemUnsubscribed( const JID& jid ) = 0;

      /**
       * Reimplement this function if you want to receive the whole server-side roster
       * on the initial roster push. After successful authentication, RosterManager asks the
       * server for the full server-side roster. Invocation of this method announces its arrival.
       * Roster item status is set to 'unavailable' until incoming presence info updates it. A full
       * roster push only happens once per connection.
       * @param roster The full roster.
       */
      virtual void handleRoster( const Roster& roster ) = 0;

      /**
       * This function is called on every status change of an item in the roster.
       * If the presence is of type Unavailable, then the resource has already been
       * removed from the RosterItem.
       * @param item The roster item.
       * @param resource The resource that changed presence.
       * @param presence The item's new presence.
       * @param msg The status change message.
       * @since 0.9
       */
      virtual void handleRosterPresence( const RosterItem& item, const std::string& resource,
                                         Presence::PresenceType presence, const std::string& msg ) = 0;

      /**
       * This function is called on every status change of a JID that matches the Client's
       * own JID.
       * If the presence is of type Unavailable, then the resource has already been
       * removed from the RosterItem.
       * @param item The self item.
       * @param resource The resource that changed presence.
       * @param presence The item's new presence.
       * @param msg The status change message.
       * @since 0.9
       */
      virtual void handleSelfPresence( const RosterItem& item, const std::string& resource,
                                       Presence::PresenceType presence, const std::string& msg ) = 0;

      /**
       * This function is called when an entity wishes to subscribe to this entity's presence.
       * If the handler is registered as a asynchronous handler for subscription requests,
       * the return value of this function is ignored. In this case you should use
       * RosterManager::ackSubscriptionRequest() to answer the request.
       * @param jid The requesting item's address.
       * @param msg A message sent along with the request.
       * @return Return @b true to allow subscription and subscribe to the remote entity's
       * presence, @b false to ignore the request.
       */
      virtual bool handleSubscriptionRequest( const JID& jid, const std::string& msg ) = 0;

      /**
       * This function is called when an entity unsubscribes from this entity's presence.
       * If the handler is registered as a asynchronous handler for subscription requests,
       * the return value of this function is ignored. In this case you should use
       * RosterManager::unsubscribe() if you want to unsubscribe yourself from the contct's
       * presence and to remove the contact from the roster.
       * @param jid The item's address.
       * @param msg A message sent along with the request.
       * @return Return @b true to unsubscribe from the remote entity, @b false to ignore.
       */
      virtual bool handleUnsubscriptionRequest( const JID& jid, const std::string& msg ) = 0;

      /**
       * This function is called whenever presence from an entity is received which is not in
       * the roster.
       * @param presence The full presence stanza.
       */
      virtual void handleNonrosterPresence( const Presence& presence ) = 0;

      /**
       * This function is called if the server returned an error.
       * @param iq The error stanza.
       */
      virtual void handleRosterError( const IQ& iq ) = 0;
  };

}

#endif // ROSTERLISTENER_H__
