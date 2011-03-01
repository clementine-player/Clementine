/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef GPGENCRYPTED_H__
#define GPGENCRYPTED_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an abstraction of a jabber:x:encrypted namespace element, as used in XEP-0027
   * (Current Jabber OpenPGP Usage).
   *
   * This class does not encrypt or decrypt any stanza content. It's meant to be an abstraction
   * of the XML representation only.
   *
   * XEP version: 1.3
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API GPGEncrypted : public StanzaExtension
  {
    public:
      /**
       * Constructs a new object with the given encrypted message.
       * @param encrypted The encrypted message.
       */
      GPGEncrypted( const std::string& encrypted );

      /**
       * Constructs an GPGEncrypted object from the given Tag. To be recognized properly, the Tag should
       * have a name of 'x' in the @c jabber:x:encrypted namespace.
       * @param tag The Tag to parse.
       */
      GPGEncrypted( const Tag* tag );

      /**
       * Virtual destructor.
       */
      virtual ~GPGEncrypted();

      /**
       * Returns the encrypted message.
       * @return The encrypted message.
       */
      const std::string& encrypted() const { return m_encrypted; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new GPGEncrypted( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new GPGEncrypted( *this );
      }

    private:
      std::string m_encrypted;
      bool m_valid;

  };

}

#endif // GPGENCRYPTED_H__
