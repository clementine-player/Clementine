/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef CHATSTATE_H__
#define CHATSTATE_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief An implementation of Chat State Notifications (XEP-0085) as a StanzaExtension.
   *
   * @author Vincent Thomasset
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API ChatState : public StanzaExtension
  {
    public:

      /**
       * Constructs a new object from the given Tag.
       * @param tag A Tag to parse.
       */
      ChatState( const Tag* tag );

      /**
       * Constructs a new object of the given type.
       * @param state The chat state.
       */
      ChatState( ChatStateType state )
        : StanzaExtension( ExtChatState ), m_state( state )
      {}

      /**
       * Virtual destructor.
       */
      virtual ~ChatState() {}

      /**
       * Returns the object's state.
       * @return The object's state.
       */
      ChatStateType state() const { return m_state; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new ChatState( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new ChatState( *this );
      }

    private:
      ChatStateType m_state;

  };

}

#endif // CHATSTATE_H__
