/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef MESSAGEEVENT_H__
#define MESSAGEEVENT_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief An implementation of Message Events (XEP-0022) as a StanzaExtension.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API MessageEvent : public StanzaExtension
  {
    public:

      /**
       * Constructs a new object from the given Tag.
       * @param tag A Tag to parse.
       */
      MessageEvent( const Tag* tag );

      /**
       * Constructs a new object of the given type, with an optional message ID.
       * @param type One or more @link gloox::MessageEventType MessageEventType @endlink.
       * @param id An optional message ID. Links this Event to the message it is generated for.
       */
      MessageEvent( int type, const std::string& id = EmptyString )
        : StanzaExtension( ExtMessageEvent ), m_id( id ), m_event( type )
      {}

      /**
       * Virtual destructor.
       */
      virtual ~MessageEvent() {}

      /**
       * Returns the object's event or events.
       * @return The object's event or events.
       */
      int event() const { return m_event; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new MessageEvent( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new MessageEvent( *this );
      }

    private:
      std::string m_id;
      int m_event;

  };

}

#endif // MESSAGEEVENT_H__
