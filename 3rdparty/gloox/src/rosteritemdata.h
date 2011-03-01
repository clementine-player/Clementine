/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef ROSTERITEMBASE_H__
#define ROSTERITEMBASE_H__

#include "gloox.h"
#include "jid.h"
#include "tag.h"

#include <string>
#include <list>


namespace gloox
{

  /**
   * @brief A class holding roster item data.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API RosterItemData
  {

    public:
      /**
       * Constructs a new item of the roster.
       * @param jid The JID of the contact.
       * @param name The displayed name of the contact.
       * @param groups A list of groups the contact belongs to.
       */
      RosterItemData( const std::string& jid, const std::string& name,
                      const StringList& groups )
        : m_jid( jid ), m_name( name ), m_groups( groups ),
          m_subscription( S10nNone ), m_changed( false ), m_remove( false )
      {}

      /**
       * Constructs a new item of the roster, scheduled for removal.
       * @param jid The JID of the contact to remove.
       */
      RosterItemData( const std::string& jid )
        : m_jid( jid ), m_subscription( S10nNone ), m_changed( false ),
          m_remove( true )
      {}

      /**
       * Virtual destructor.
       */
      virtual ~RosterItemData() {}

      /**
       * Returns the contact's bare JID.
       * @return The contact's bare JID.
       */
      const std::string& jid() const { return m_jid; }

      /**
       * Sets the displayed name of a contact/roster item.
       * @param name The contact's new name.
       */
      void setName( const std::string& name )
      {
        m_name = name;
        m_changed = true;
      }

      /**
       * Retrieves the displayed name of a contact/roster item.
       * @return The contact's name.
       */
      const std::string& name() const { return m_name; }

      /**
       * Sets the current subscription status of the contact.
       * @param subscription The current subscription.
       * @param ask Whether a subscription request is pending.
       */
      void setSubscription( const std::string& subscription, const std::string& ask )
      {
        m_sub = subscription;
        m_ask = ask;

        if( subscription == "from" && ask.empty() )
          m_subscription = S10nFrom;
        else if( subscription == "from" && !ask.empty() )
          m_subscription = S10nFromOut;
        else if( subscription == "to" && ask.empty() )
          m_subscription = S10nTo;
        else if( subscription == "to" && !ask.empty() )
          m_subscription = S10nToIn;
        else if( subscription == "none" && ask.empty() )
          m_subscription = S10nNone;
        else if( subscription == "none" && !ask.empty() )
          m_subscription = S10nNoneOut;
        else if( subscription == "both" )
          m_subscription = S10nBoth;
      }

      /**
       * Returns the current subscription type between the remote and the local entity.
       * @return The subscription type.
       */
      SubscriptionType subscription() const { return m_subscription; }

      /**
       * Sets the groups this RosterItem belongs to.
       * @param groups The groups to set for this item.
       */
      void setGroups( const StringList& groups )
      {
        m_groups = groups;
        m_changed = true;
      }

      /**
       * Returns the groups this RosterItem belongs to.
       * @return The groups this item belongs to.
       */
      const StringList& groups() const { return m_groups; }

      /**
       * Whether the item has unsynchronized changes.
       * @return @b True if the item has unsynchronized changes, @b false otherwise.
       */
      bool changed() const { return m_changed; }

      /**
       * Whether the item is scheduled for removal.
       * @return @b True if the item is subject to a removal or scheduled for removal, @b false
       * otherwise.
       */
      bool remove() const { return m_remove; }

      /**
       * Removes the 'changed' flag from the item.
       */
      void setSynchronized() { m_changed = false; }

      /**
       * Retruns a Tag representation of the roster item data.
       * @return A Tag representation.
       */
      Tag* tag() const
      {
        Tag* i = new Tag( "item" );
        i->addAttribute( "jid", m_jid );
        if( m_remove )
          i->addAttribute( "subscription", "remove" );
        else
        {
          i->addAttribute( "name", m_name );
          StringList::const_iterator it = m_groups.begin();
          for( ; it != m_groups.end(); ++it )
            new Tag( i, "group", (*it) );
          i->addAttribute( "subscription", m_sub );
          i->addAttribute( "ask", m_ask );
        }
        return i;
      }

    protected:
      std::string m_jid;
      std::string m_name;
      StringList m_groups;
      SubscriptionType m_subscription;
      std::string m_sub;
      std::string m_ask;
      bool m_changed;
      bool m_remove;

  };

}

#endif // ROSTERITEMBASE_H__
