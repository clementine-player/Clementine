/*
  Copyright (c) 2008-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef EVENT_H__
#define EVENT_H__

namespace gloox
{

  class Stanza;

  /**
   * @brief A base class for events.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class Event
  {

    public:
      /**
       * Event types.
       */
      enum EventType
      {
        PingPing,                     /**< Incoming Ping (XEP-0199). */
        PingPong,                     /**< Incoming Pong (XEP-0199). */
        PingError                     /**< Incoming Error Pong (XEP-0199). */
      };

      /**
       * Creates a new Event of the given type.
       * @param type The Event type.
       */
      Event( EventType type ) : m_eventType( type ), m_stanza( 0 ) {}

      /**
       * Creates a new Event of the given type, referencing the given Stanza.
       * @param type The Event type.
       * @param stanza A Stanza to point at. No copy of the Stanza is taken, just its address.
       */
      Event( EventType type, const Stanza& stanza ) : m_eventType( type ), m_stanza( &stanza ) {}

      /**
       * Virtual Destructor.
       */
      virtual ~Event() {}

      /**
       * Returns the Event's type.
       * @return The Event's type.
       */
      EventType eventType() const { return m_eventType; }

      /**
       * Returns a pointer to a Stanza-derived object.
       * @return A pointer to a Stanza that caused the event. May be 0.
       * @note You should @b not delete the Stanza object.
       */
      const Stanza* stanza() const { return m_stanza; }

    protected:
      EventType m_eventType;
      const Stanza* m_stanza;

  };

}

#endif // EVENT_H__
