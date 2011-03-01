/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef NICKNAME_H__
#define NICKNAME_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief An implementation of User Nickname (XEP-0172) as a StanzaExtension.
   *
   * XEP version: 1.0
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Nickname : public StanzaExtension
  {
    public:

      /**
       * Constructs a new object from the given Tag.
       * @param tag A Tag to parse.
       */
      Nickname( const Tag* tag );

      /**
       * Constructs a new Nickname object.
       * @param nick The nickname to include.
       */
      Nickname( const std::string& nick )
        : StanzaExtension( ExtNickname ), m_nick( nick )
      {}

      /**
       * Virtual destructor.
       */
      virtual ~Nickname() {}

      /**
       * Returns the extension's saved nickname.
       * @return The nickname.
       */
      const std::string nick() const { return m_nick; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new Nickname( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new Nickname( *this );
      }

    private:
      std::string m_nick;

  };

}

#endif // NICKNAME_H__
