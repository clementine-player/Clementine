/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef SUBSCRIPTION_H__
#define SUBSCRIPTION_H__

#include "stanza.h"

#include <string>

namespace gloox
{

  class JID;

  /**
   * @brief An abstraction of a subscription stanza.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Subscription : public Stanza
  {

    public:

      friend class ClientBase;

      /**
       * Describes the different valid message types.
       */
      enum S10nType
      {
        Subscribe,                  /**> A subscription request. */
        Subscribed,                 /**< A subscription notification. */
        Unsubscribe,                /**< An unsubscription request. */
        Unsubscribed,               /**< An unsubscription notification. */
        Invalid                     /**< The stanza is invalid. */
      };

      /**
       * Creates a Subscription request.
       * @param type The presence type.
       * @param to The intended receiver. Use an empty JID to create a broadcast packet.
       * @param status An optional status message (e.g. "please authorize me").
       * @param xmllang An optional xml:lang for the status message.
       */
      Subscription( S10nType type, const JID& to, const std::string& status = EmptyString,
                     const std::string& xmllang = EmptyString );
      /**
       * Destructor.
       */
      virtual ~Subscription();

      /**
       * Returns the subscription stanza's type.
       * @return The subscription stanza's type.
       *
       */
      S10nType subtype() const { return m_subtype; }

      /**
       * Returns the status text of a presence stanza for the given language if available.
       * If the requested language is not available, the default status text (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default body
       * will be returned, if any.
       * @return The status text set by the sender.
       */
      const std::string status( const std::string& lang = "default" ) const
      {
        return findLang( m_stati, m_status, lang );
      }

      // reimplemented from Stanza
      virtual Tag* tag() const;

    private:
#ifdef SUBSCRIPTION_TEST
    public:
#endif
      /**
       * Creates a Subscription request from the given Tag. The original Tag will be ripped off.
       * @param tag The Tag to parse.
       */
      Subscription( Tag* tag );

      S10nType m_subtype;
      StringMap* m_stati;
      std::string m_status;

  };

}

#endif // SUBSCRIPTION_H__
