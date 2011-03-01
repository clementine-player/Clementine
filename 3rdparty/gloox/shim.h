/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef SHIM_H__
#define SHIM_H__

#include "stanzaextension.h"
#include "macros.h"

#include <map>
#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief An implementation/abstraction of Stanza Headers and Internet Metadata (SHIM, XEP-0131).
   *
   * XEP Version: 1.2
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API SHIM : public StanzaExtension
  {
    public:
      /**
       * A list of SHIM headers (name & value).
       */
      typedef std::map<std::string, std::string> HeaderList;

      /**
       * Creates a new SHIM object containing the given headers.
       * @param hl The list of headers.
       */
      SHIM( const HeaderList& hl );

      /**
       * Creates a new SHIM object from the given Tag.
       * @param tag The Tag to parse.
       */
      SHIM( const Tag* tag = 0 );

      /**
       * Returns the headers.
       * @return The headers.
       */
      const HeaderList& headers() const { return m_headers; }

      /**
       * Virtual destructor.
       */
      virtual ~SHIM();

      // re-implemented from StanzaExtension
      virtual const std::string& filterString() const;

      // re-implemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new SHIM( tag );
      }

      // re-implemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new SHIM( *this );
      }

    private:
      HeaderList m_headers;

  };

}

#endif // SHIM_H__
