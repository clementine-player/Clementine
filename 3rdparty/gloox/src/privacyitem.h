/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef PRIVACYITEM_H__
#define PRIVACYITEM_H__

#include "macros.h"
#include "gloox.h"

#include <string>

namespace gloox
{

  /**
   * @brief This is an abstraction of a single item of a privacy list, describing an allowed or
   * forbidden action.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API PrivacyItem
  {
    public:

      /**
       * Three possible types of an item. Only one is allowed at a time.
       */
      enum ItemType
      {
        TypeUndefined,              /**< None of the types below is explicitely selected,
                                     * &quot;fall-through&quot; case. */
        TypeJid,                    /**< The item affects the JID which is given in the value attribute. */
        TypeGroup,                  /**< The item affects the group which is given in the value attribute and
                                     * which must exist at least once in the users roster. */
        TypeSubscription            /**< The item affects the subscription type which is given in the value
                                     * attribute. */
      };

      /**
       * Two possible actions. Only one is allowed at a time.
       */
      enum ItemAction
      {
        ActionAllow,                /**< The item explicitely allows the described packets. */
        ActionDeny                  /**< The item forbids the described packets. */
      };

      /**
       * The packet type a privacy item affects (blocks). Combinations are allowed.
       */
      enum ItemPacketType
      {
        PacketMessage        =  1,  /**< The item blocks message stanzas. */
        PacketPresenceIn     =  2,  /**< The item blocks incoming presence stanzas. */
        PacketPresenceOut    =  4,  /**< The item blocks outgoing presence stanzas. */
        PacketIq             =  8,  /**< The item blocks IQ stanzas. */
        PacketAll            = 15   /**< The item blocks all of these stanza types. */
      };

      /**
       * Constructs a new privacy item.
       * @param type Action is based on matching JID, Group or Subscription.
       * @param action The action to carry out. (Deny or allow.)
       * @param packetType Affected packet types. Bit-wise OR'ed ItemPacketType.
       * @param value The value to check for and match.
       */
      PrivacyItem( const ItemType type = TypeUndefined, const ItemAction action = ActionAllow,
                   const int packetType = 0, const std::string& value = EmptyString );

      /**
       * Virtual destructor.
       */
      virtual ~PrivacyItem();

      /**
       * Returns the item type.
       * @return The type of the item.
       */
      ItemType type() const { return m_type; }

      /**
       * Returns the item's action.
       * @return The action of the item.
       */
      ItemAction action() const { return m_action; }

      /**
       * Returns the packet type the item affects.
       * @return An OR'ed list of affected packet types.
       */
      int packetType() const { return m_packetType; }

      /**
       * Returns the value of the item's 'value' attribute.
       * @return value The 'value' attribute's value.
       */
      const std::string value() const { return m_value; }

      /**
       * Compares the current PrivacyItem with another one.
       * @param item The item which shall be compared.
       * @return @b True if both items are equal, @b false otherwise.
       */
      bool operator==( const PrivacyItem& item ) const;

    private:
      ItemType m_type;
      ItemAction m_action;
      int m_packetType;
      std::string m_value;
  };

}

#endif // PRIVACYITEM_H__
