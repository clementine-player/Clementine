/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef GPGSIGNED_H__
#define GPGSIGNED_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an abstraction of a jabber:x:signed namespace element, as used in XEP-0027
   * (Current Jabber OpenPGP Usage).
   *
   * This class does not sign or verify any stanza content. It's meant to be an abstraction
   * of the XML representation only.
   *
   * XEP version: 1.3
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API GPGSigned : public StanzaExtension
  {
    public:
      /**
       * Constructs a new object with the given signature.
       * @param signature The signature.
       */
      GPGSigned( const std::string& signature );

      /**
       * Constructs an GPGSigned object from the given Tag. To be recognized properly, the Tag should
       * have a name of 'x' in the @c jabber:x:signed namespace.
       * @param tag The Tag to parse.
       */
      GPGSigned( const Tag* tag );

      /**
       * Virtual destructor.
       */
      virtual ~GPGSigned();

      /**
       * Returns the signature.
       * @return The signature.
       */
      const std::string& signature() const { return m_signature; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new GPGSigned( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new GPGSigned( *this );
      }

    private:
      std::string m_signature;
      bool m_valid;

  };

}

#endif // GPGSIGNED_H__
