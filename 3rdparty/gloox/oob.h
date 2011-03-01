/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef OOB_H__
#define OOB_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an abstraction of a jabber:x:oob namespace element or a jabber:iq:oob namespace element
   * as specified in XEP-0066.
   *
   * XEP version: 1.5
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API OOB : public StanzaExtension
  {
    public:
      /**
       * Constructs an OOB StanzaExtension from teh given URL and description.
       * @param url The out-of-band URL.
       * @param description The URL's optional description.
       * @param iqext Whether this object extends an IQ or a Presence/Message stanza (results in
       * either jabber:iq:oob or jabber:x:oob namespaced element).
       */
      OOB( const std::string& url, const std::string& description, bool iqext );

      /**
       * Constructs an OOB object from the given Tag. To be recognized properly, the Tag must
       * have either a name of 'x' in the jabber:x:oob namespace, or a name of 'query' in the
       * jabber:iq:oob namespace.
       * @param tag The Tag to parse.
       */
      OOB( const Tag* tag );

      /**
       * Virtual destructor.
       */
      virtual ~OOB();

      /**
       * Returns the out-of-band URL.
       * @return The out-of-band URL.
       */
      const std::string& url() const { return m_url; }

      /**
       * Returns the URL's description.
       * @return The URL's description.
       */
      const std::string& desc() const { return m_desc; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new OOB( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new OOB( *this );
      }

    private:
      std::string m_xmlns;
      std::string m_url;
      std::string m_desc;
      bool m_iqext;
      bool m_valid;

  };

}

#endif // OOB_H__
