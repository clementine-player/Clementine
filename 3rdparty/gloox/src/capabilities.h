/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CAPABILITIES_H__
#define CAPABILITIES_H__

#include "disconodehandler.h"
#include "stanzaextension.h"
#include "tag.h"

#include <string>

namespace gloox
{

  class Disco;
  class Tag;

  /**
   * @brief This is an implementation of XEP-0115 (Entity Capabilities).
   *
   * XEP Version: 1.5-15
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Capabilities : public StanzaExtension, public DiscoNodeHandler
  {

    public:
      /**
       * Constructs a new object and fills it according to the parameters.
       * @param disco The current Client's Disco object.
       */
      Capabilities( Disco* disco );

      /**
       * Constructs a new object from the given Tag.
       * @param tag The Tag to parse.
       */
      Capabilities( const Tag* tag = 0 );

      /**
       * Virtual Destructor.
       */
      virtual ~Capabilities();

      /**
       * Returns the client's identifying node.
       * @return The node.
       */
      const std::string& node() const { return m_node; }

      /**
       * Sets the client's identifying node.
       * @param node The node.
       */
      void setNode( const std::string& node ) { m_node = node; }

      /**
       * Returns the client's identifying ver string.
       * @return The ver string.
       */
      const std::string ver() const;

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new Capabilities( tag );
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new Capabilities( *this );
      }

      // reimplemented from DiscoNodeHandler
      virtual StringList handleDiscoNodeFeatures( const JID& from, const std::string& node );

      // reimplemented from DiscoNodeHandler
      virtual Disco::IdentityList handleDiscoNodeIdentities( const JID& from,
                                                             const std::string& node );

      // reimplemented from DiscoNodeHandler
      virtual Disco::ItemList handleDiscoNodeItems( const JID& from, const JID& to,
                                                    const std::string& node = EmptyString );

    private:
      /**
       * Returns the hash function used for creating the caps info.
       * @return The current hash function's name.
       */
      const std::string& hash() const { return m_hash; }

      /**
       * Use this function to set the hash function to use.
       * @param hash The hash function.
       * @todo Convert to using an enum and make public.
       */
      void setHash( const std::string& hash ) { m_hash = hash; }

      static std::string generate( const Disco::IdentityList& identities,
                                   const StringList& features, const DataForm* form = 0 );
      static std::string generate( const Disco::Info* info );
      static std::string generate( const Disco* disco );

      Disco* m_disco;
      std::string m_node;
      std::string m_hash;
      std::string m_ver;
      bool m_valid;
  };

}

#endif // CAPABILITIES_H__
