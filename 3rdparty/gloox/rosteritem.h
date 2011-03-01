/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef ROSTERITEM_H__
#define ROSTERITEM_H__

#include "gloox.h"
#include "resource.h"
#include "presence.h"

#include <string>
#include <list>


namespace gloox
{

  class RosterItemData;

  /**
   * @brief An abstraction of a roster item.
   *
   * For each RosterItem all resources that are available (online in some way) are stored in
   * a ResourceMap. This map is accessible using the resources() function.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API RosterItem
  {
    friend class RosterManager;

    public:
      /**
       * A list of resources for the given JID.
       */
      typedef std::map<std::string, Resource*> ResourceMap;

      /**
       * Constructs a new item of the roster.
       * @param jid The JID of the contact.
       * @param name The displayed name of the contact.
       */
      RosterItem( const std::string& jid, const std::string& name = EmptyString );

      /**
       * Constructs a new RosterItem using the data holding class.
       * @param data The RosterItemData to construct the item from. The new
       * item will own the data object.
       */
      RosterItem( const RosterItemData& data );

      /**
       * Virtual destructor.
       */
      virtual ~RosterItem();

      /**
       * Sets the displayed name of a contact/roster item.
       * @param name The contact's new name.
       */
      void setName( const std::string& name );

      /**
       * Retrieves the displayed name of a contact/roster item.
       * @return The contact's name.
       */
      const std::string& name() const;

      /**
       * Returns the contact's bare JID.
       * @return The contact's bare JID.
       */
      const std::string& jid() const;

      /**
       * Sets the current subscription status of the contact.
       * @param subscription The current subscription.
       * @param ask Whether a subscription request is pending.
       */
      void setSubscription( const std::string& subscription, const std::string& ask );

      /**
       * Returns the current subscription type between the remote and the local entity.
       * @return The subscription type.
       */
      SubscriptionType subscription() const;

      /**
       * Sets the groups this RosterItem belongs to.
       * @param groups The groups to set for this item.
       */
      void setGroups( const StringList& groups );

      /**
       * Returns the groups this RosterItem belongs to.
       * @return The groups this item belongs to.
       */
      const StringList groups() const;

      /**
       * Whether the item has unsynchronized changes.
       * @return @b True if the item has unsynchronized changes, @b false otherwise.
       */
      bool changed() const;

      /**
       * Indicates whether this item has at least one resource online (in any state).
       * @return @b True if at least one resource is online, @b false otherwise.
       */
      bool online() const;

      /**
       * Returns the contact's resources.
       * @return The contact's resources.
       */
      const ResourceMap& resources() const { return m_resources; }

      /**
       * Returns the Resource for a specific resource string.
       * @param res The resource string.
       * @return The Resource if found, 0 otherwise.
       */
      const Resource* resource( const std::string& res ) const;

      /**
       * Returns the Resource with the highest priority.
       * @return The Resource with the highest priority.
       */
      const Resource* highestResource() const;

    protected:
      /**
       * Sets the current presence of the resource.
       * @param resource The resource to set the presence for.
       * @param presence The current presence.
       */
      void setPresence( const std::string& resource, Presence::PresenceType presence );

      /**
       * Sets the current status message of the resource.
       * @param resource The resource to set the status message for.
       * @param msg The current status message, i.e. from the presence info.
       */
      void setStatus( const std::string& resource, const std::string& msg );

      /**
       * Sets the current priority of the resource.
       * @param resource The resource to set the status message for.
       * @param priority The resource's priority, i.e. from the presence info.
       */
      void setPriority( const std::string& resource, int priority );

      /**
       * Sets the resource's presence extensions.
       * @param resource The resource to set the extensions for.
       * @param exts The extensions to set.
       */
      void setExtensions( const std::string& resource, const StanzaExtensionList& exts );

      /**
       * Removes the 'changed' flag from the item.
       */
      void setSynchronized();

      /**
       * This function is called to remove subsequent resources from a RosterItem.
       * @param resource The resource to remove.
       */
      void removeResource( const std::string& resource );

      /**
       * This function deletes the internal RosterItemData and replaces it with the provided
       * one. The RosterItem will own the RosterItemData instance.
       */
      void setData( const RosterItemData& rid );

    private:
      RosterItemData* m_data;
      ResourceMap m_resources;

  };

}

#endif // ROSTERITEM_H__
