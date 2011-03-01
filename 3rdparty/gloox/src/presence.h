/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef PRESENCE_H__
#define PRESENCE_H__

#include "stanza.h"

#include <string>

namespace gloox
{

  class Capabilities;
  class JID;

  /**
   * @brief An abstraction of a presence stanza.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Presence : public Stanza
  {

    friend class ClientBase;

    public:

      /**
       * Describes the different valid presence types.
       */
      enum PresenceType
      {
        Available,                  /**< The entity is online. */
        Chat,                       /**< The entity is 'available for chat'. */
        Away,                       /**< The entity is away. */
        DND,                        /**< The entity is DND (Do Not Disturb). */
        XA,                         /**< The entity is XA (eXtended Away). */
        Unavailable,                /**< The entity is offline. */
        Probe,                      /**< This is a presence probe. */
        Error,                      /**< This is a presence error. */
        Invalid                     /**< The stanza is invalid. */
      };

      /**
       * Creates a Presence request.
       * @param type The presence type.
       * @param to The intended receiver. Use an empty JID to create a broadcast packet.
       * @param status An optional status message (e.g. "gone fishing").
       * @param priority An optional presence priority. Legal range is between -128 and +127.
       * Defaults to 0.
       * @param xmllang An optional xml:lang for the status message.
       */
      Presence( PresenceType type, const JID& to, const std::string& status = EmptyString,
                int priority = 0, const std::string& xmllang = EmptyString );

      /**
       * Destructor.
       */
      virtual ~Presence();

      /**
       * Returns the presence's type.
       * @return The presence's type.
       */
      PresenceType subtype() const { return m_subtype; }

      /**
       * A convenience function returning the stanza's Capabilities, if any. May be 0.
       * @return A pointer to a Capabilities object, or 0.
       */
      const Capabilities* capabilities() const;

      /**
       * Returns the presence's type.
       * @return The presence's type.
       */
//#warning FIXME return something useful (only 'show' values?) or kill this func
      PresenceType presence() const { return m_subtype; }

      /**
       * Sets the presence type.
       * @param type The presence type.
       */
      void setPresence( PresenceType type ) { m_subtype = type; }

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

      /**
       * Adds a (possibly translated) status message.
       * @param status The status message.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066.
       */
      void addStatus( const std::string& status, const std::string& lang = EmptyString )
      {
        setLang( &m_stati, m_status, status, lang );
      }

      /**
       * Resets the default status message as well as all language-specific ones.
       */
      void resetStatus();

      /**
       * Returns the presence priority in the legal range: -128 to +127.
       * @return The priority information contained in the stanza, defaults to 0.
       */
      int priority() const { return m_priority; }

      /**
       * Sets the priority. Legal range: -128 to +127.
       * @param priority The priority to set.
       */
      void setPriority( int priority );

      // reimplemented from Stanza
      virtual Tag* tag() const;

    private:
#ifdef PRESENCE_TEST
    public:
#endif
      /**
       * Creates a Presence request from the given Tag. The original Tag will be ripped off.
       * @param tag The Tag to parse.
       */
      Presence( Tag* tag );

      PresenceType m_subtype;
      StringMap* m_stati;
      std::string m_status;
      int m_priority;

  };

}

#endif // PRESENCE_H__
