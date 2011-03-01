/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef ROSTERMANAGER_H__
#define ROSTERMANAGER_H__

#include "subscriptionhandler.h"
#include "privatexmlhandler.h"
#include "iqhandler.h"
#include "presencehandler.h"
#include "rosterlistener.h"

#include <map>
#include <string>
#include <list>

namespace gloox
{

  class ClientBase;
  class Stanza;
  class PrivateXML;
  class RosterItem;

  /**
   * @brief This class implements Jabber/XMPP roster handling in the @b jabber:iq:roster namespace.
   *
   * It takes care of changing presence, subscriptions, etc.
   * You can modify any number of RosterItems within the Roster at any time. These changes must be
   * synchronized with the server by calling @ref synchronize(). Note that incoming Roster pushes
   * initiated by other resources may overwrite changed values.
   * Additionally, XEP-0083 (Nested Roster Groups) is implemented herein.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
   class GLOOX_API RosterManager : public IqHandler, public PresenceHandler, public SubscriptionHandler,
                                   public PrivateXMLHandler
  {
    public:
      /**
       * Creates a new RosterManager.
       * @param parent The ClientBase which is used for communication.
       */
      RosterManager( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~RosterManager();

      /**
       * This function does the initial filling of the roster with
       * the current server-side roster.
       */
      void fill();

      /**
       * This function returns the roster.
       * @return Returns a map of JIDs with their current presence.
       */
      Roster* roster();

      /**
       * Use this function to subscribe to a new JID. The contact is added to the roster automatically
       * (by compliant servers, as required by RFC 3921).
       * @param jid The address to subscribe to.
       * @param name The displayed name of the contact.
       * @param groups A list of groups the contact belongs to.
       * @param msg A message sent along with the request.
       */
      void subscribe( const JID& jid, const std::string& name = EmptyString,
                      const StringList& groups = StringList(),
                      const std::string& msg = EmptyString );

      /**
       * Synchronizes locally modified RosterItems back to the server.
       */
      void synchronize();

      /**
       * Use this function to add a contact to the roster. No subscription request is sent.
       * @note Use @ref unsubscribe() to remove an item from the roster.
       * @param jid The JID to add.
       * @param name The displayed name of the contact.
       * @param groups A list of groups the contact belongs to.
       */
      void add( const JID& jid, const std::string& name, const StringList& groups );

      /**
       * Use this function to unsubscribe from a contact's presence. You will no longer
       * receive presence from this contact.
       * @param jid The address to unsubscribe from.
       * @param msg A message to send along with the request.
       * @since 0.9
       * @note Use remove() to remove a contact from the roster and to cancel its subscriptions.
       */
      void unsubscribe( const JID& jid, const std::string& msg = EmptyString );

      /**
       * Use this function to cancel the contact's subscription to your presence. The contact will
       * no longer receive presence from you.
       * @param jid The contact's JID.
       * @param msg A message to send along with the request.
       * @since 0.9
       * @note Use remove() to remove a contact from the roster and to cancel its subscriptions.
       */
      void cancel( const JID& jid, const std::string& msg = EmptyString );

      /**
       * Use this function to remove a contact from the roster. Subscription is implicitely
       * cancelled.
       * @param jid The contact's JID.
       * @since 0.9
       */
      void remove( const JID& jid );

      /**
       * Use this function to acknowledge a subscription request if you requested asynchronous
       * subscription request handling.
       * @param to The JID to authorize/decline.
       * @param ack Whether to authorize or decline the contact's request.
       */
      void ackSubscriptionRequest( const JID& to, bool ack );

      /**
       * Use this function to retrieve the delimiter of Nested Roster Groups (XEP-0083).
       * @return The group delimiter.
       * @since 0.7
       */
      const std::string& delimiter() const { return m_delimiter; }

      /**
       * Use this function to set the group delimiter (XEP-0083).
       * @param delimiter The group delimiter.
       * @since 0.7
       */
      void setDelimiter( const std::string& delimiter );

      /**
       * Lets you retrieve the RosterItem that belongs to the given JID.
       * @param jid The JID to return the RosterItem for.
       */
      RosterItem* getRosterItem( const JID& jid );

      /**
       * Register @c rl as object that receives updates on roster operations.
       * For GUI applications it may be necessary to display a dialog or whatever to
       * the user without blocking. If you want that, use asynchronous subscription
       * requests. If you want to answer a request right away, make it synchronous.
       * @param rl The object that receives roster updates.
       * @param syncSubscribeReq Indicates whether (Un)SubscriptionRequests shall
       * be handled synchronous (@b true) or asynchronous (@b false). Default: synchronous.
       */
      void registerRosterListener( RosterListener* rl, bool syncSubscribeReq = true );

      /**
       * Complementary function to @ref registerRosterListener. Removes the current RosterListener.
       * Roster events will not be delivered anywhere.
       */
      void removeRosterListener();

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

      // reimplemented from PresenceHandler.
      virtual void handlePresence( const Presence& presence );

      // reimplemented from SubscriptionHandler.
      virtual void handleSubscription( const Subscription& subscription );

      // reimplemented from PrivateXMLHandler
      virtual void handlePrivateXML( const Tag* xml );

      // reimplemented from PrivateXMLHandler
      virtual void handlePrivateXMLResult( const std::string& uid, PrivateXMLResult pxResult );

    private:
#ifdef ROSTERMANAGER_TEST
    public:
#endif
      typedef std::list<RosterItemData*> RosterData;

      /**
       * @brief An implementation of StanzaExtension that helps in roster management.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class Query : public StanzaExtension
      {
        public:
          /**
           * Constructs a new object that can be used to add a contact to the roster.
           * @param jid The contact's JID.
           * @param name The contact's optional user-defined name.
           * @param groups An optional list of groups the contact belongs to.
           */
          Query( const JID& jid, const std::string& name, const StringList& groups );

          /**
           * Constructs a new object that can be used to remove a contact from the roster.
           * @param jid The contact's JID.
           */
          Query( const JID& jid );

          /**
           * Creates a new Query object from teh given Tag.
           * @param tag The Tag to parse.
           */
          Query( const Tag* tag = 0 );

          /**
           * Destructor.
           */
          ~Query();

          /**
           * Retruns the internal roster that was created by the ctors (either from an
           * incoming packet or passed arguments).
           * This is not necessarily the full roster, but may be a single item.
           * @return The (possibly partial) roster).
           */
          const RosterData& roster() const { return m_roster; }

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
          virtual StanzaExtension* clone() const;

        private:
          RosterData m_roster;

      };

      void mergePush( const RosterData& data );
      void mergeRoster( const RosterData& data );

      RosterListener* m_rosterListener;
      Roster m_roster;
      ClientBase* m_parent;
      PrivateXML* m_privateXML;
      RosterItem* m_self;

      std::string m_delimiter;
      bool m_syncSubscribeReq;

      enum RosterContext
      {
        RequestRoster,
        AddRosterItem,
        RemoveRosterItem,
        SynchronizeRoster
      };

  };

}

#endif // ROSTER_H__
