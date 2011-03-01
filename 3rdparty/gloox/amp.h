/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef AMP_H__
#define AMP_H__

#include "stanzaextension.h"
#include "jid.h"

#include <string>
#include <list>

#include <ctime>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an implementation of XEP-0079 (Advanced Message Processing)
   * as a StanzaExtension.
   *
   * XEP Version: 1.2
   * @author Jakob Schroeter <js@camaya.net>
   * @author Vincent Thomasset
   * @since 1.0
   */
  class GLOOX_API AMP : public StanzaExtension
  {

    public:
      /**
       * Possible types for a rule's condition.
       */
      enum ConditionType
      {
        ConditionDeliver,           /**< Ensures (non-)delivery of the message */
        ConditionExpireAt,          /**< Ensures delivery only before a certain time (UTC) */
        ConditionMatchResource,     /**< Ensures delivery only to a specific resource type */
        ConditionInvalid            /**< Invalid condition */
      };

      /**
       * Possible actions to take when the corresponding condition is met.
       */
      enum ActionType
      {

        ActionAlert,                /**< Sends back a message stanza with an 'alert' status */
        ActionError,                /**< Sends back a message stanza with an error type */
        ActionDrop,                 /**< Silently ignore the message */
        ActionNotify,               /**< Sends back a message stanza with a 'notify' status */
        ActionInvalid               /**< Invalid action */
      };

      /**
       * Possible delivery rules.
       */
      enum DeliverType
      {
        DeliverDirect,              /**< The message would be immediately delivered to the intended
                                     * recipient or routed to the next hop. */
        DeliverForward,             /**< The message would be forwarded to another XMPP address or
                                     * account. */
        DeliverGateway,             /**< The message would be sent through a gateway to an address
                                     * or account on a non-XMPP system. */
        DeliverNone,                /**< The message would not be delivered at all (e.g., because
                                     * the intended recipient is offline and message storage is
                                     * not enabled). */
        DeliverStored,              /**< The message would be stored offline for later delivery
                                     * to the intended recipient. */
        DeliverInvalid              /**< Invalid deliver value */
      };

      /**
       * Possible resource matching rules.
       */
      enum MatchResourceType
      {
        MatchResourceAny,           /**< Destination resource matches any value, effectively
                                     * ignoring the intended resource. */
        MatchResourceExact,         /**< Destination resource exactly matches the intended
                                     * resource. */
        MatchResourceOther,         /**< Destination resource matches any value except for
                                     * the intended resource. */
        MatchResourceInvalid        /**< Invalid match-resource value */
      };

      /**
       * Available Stati.
       */
      enum Status
      {
        StatusAlert,                      /**< The message is a reply to a @c Alert rule. */
        StatusNotify,                     /**< The message is a reply to a @c Notify rule. */
        StatusInvalid                     /**< Invalid status. */
      };

      /**
       * Describes an AMP rule.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class GLOOX_API Rule
      {
        public:
          /**
           * Creates a new AMP rule object with a condition of 'deliver'.
           * @param deliver The delivery type.
           * @param action The rule's action.
           */
          Rule( DeliverType deliver, ActionType action );

          /**
           * Creates a new AMP rule object with a condition of 'expire-at'.
           * @param date The expiry date/time in the format defined in XEP-0082.
           * @param action The rule's action.
           */
          Rule( const std::string& date, ActionType action );

          /**
           * Creates a new AMP rule object with a condition of 'match-resource'.
           * @param match The match type.
           * @param action The rule's action.
           */
          Rule( MatchResourceType match, ActionType action );

          /**
           * Creates a new AMP rule object from the given strings.
           * @param condition The rule's condition.
           * @param action The rule's action.
           * @param value The rule's value.
           */
          Rule( const std::string& condition, const std::string& action,
                const std::string& value );

          /**
           * Destructor.
           */
          ~Rule();

          /**
           * Creates a Tag representation from the current rule.
           * @return A Tag representation of the rule.
           */
          Tag* tag() const;

        private:
          ConditionType m_condition;
          union
          {
            DeliverType m_deliver;
            MatchResourceType m_matchresource;
            std::string* m_expireat;
          };
          ActionType m_action;

      };

      /**
       * A list of AMP rules.
       */
      typedef std::list<const Rule*> RuleList;

      /**
       * Constructs a new object.
       * @param perhop Indicates whether the ruleset should be applied to all hops,
       * or at the edge servers only. Default: @c false (edge servers only)
       */
      AMP( bool perhop = false );

      /**
       * Constructs a new object from the given Tag.
       * @param tag The AMP Tag to parse.
       */
      AMP( const Tag* tag );

      /**
       * Adds the given rule to the list of rules.
       * @param rule The rule to add.
       */
      void addRule( const Rule* rule );

      /**
       * Returns the current list of rules for inspection.
       * @return The current list of rules.
       */
      const RuleList& rules() const { return m_rules; }

      /**
       * @brief Virtual Destructor.
       */
      virtual ~AMP();

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new AMP( tag );
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        AMP* a = new AMP();
        a->m_perhop = m_perhop;
        RuleList::const_iterator it = m_rules.begin();
        for( ; it != m_rules.end(); ++it )
          a->m_rules.push_back( new Rule( *(*it) ) );
        a->m_status = m_status;
        a->m_from = m_from;
        a->m_to = m_to;
        return a;
      }

    private:
      bool m_perhop;
      RuleList m_rules;
      Status m_status;
      JID m_from;
      JID m_to;
  };

}

#endif // AMP_H__
