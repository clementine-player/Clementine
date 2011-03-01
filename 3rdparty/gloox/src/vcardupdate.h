/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef VCARDUPDATE_H__
#define VCARDUPDATE_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an abstraction of a vcard-temp:x:update namespace element, as used in XEP-0153
   * (vCard-Based Avatars).
   *
   * XEP version: 1.0
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API VCardUpdate : public StanzaExtension
  {
    public:
      /**
       * Constructs an empty VCardUpdate object.
       */
      VCardUpdate();

      /**
       * Constructs a new object with the given hash.
       * @param hash The current avatar's SHA hash.
       */
      VCardUpdate( const std::string& hash );

      /**
       * Constructs an VCardUpdate object from the given Tag. To be recognized properly, the Tag should
       * have a name of 'x' in the @c vcard-temp:x:update namespace.
       * @param tag The Tag to parse.
       */
      VCardUpdate( const Tag* tag );

      /**
       * Virtual destructor.
       */
      virtual ~VCardUpdate();

      /**
       * Returns the avatar's hash.
       * @return The avatar's SHA hash.
       */
      const std::string& hash() const { return m_hash; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new VCardUpdate( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new VCardUpdate( *this );
      }

    private:
      std::string m_hash;
      bool m_notReady;
      bool m_noImage;
      bool m_valid;

  };

}

#endif // VCARDUPDATE_H__
