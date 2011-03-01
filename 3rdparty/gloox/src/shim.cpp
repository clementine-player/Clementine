/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "shim.h"
#include "tag.h"

namespace gloox
{

  SHIM::SHIM( const HeaderList& hl )
    : StanzaExtension( ExtSHIM ), m_headers( hl )
  {
  }

  SHIM::SHIM( const Tag* tag )
    : StanzaExtension( ExtSHIM )
  {
    if( !tag || tag->name() != "headers" || tag->xmlns() != XMLNS_SHIM )
      return;

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() != "header" || !(*it)->hasAttribute( "name" ) )
        return;

      m_headers.insert( std::make_pair( (*it)->findAttribute( "name" ), (*it)->cdata() ) );
    }
  }

  SHIM::~SHIM()
  {
  }

  const std::string& SHIM::filterString() const
  {
    static const std::string filter = "/presence/headers[@xmlns='" + XMLNS_SHIM + "']"
                                      "|/message/headers[@xmlns='" + XMLNS_SHIM + "']"
                                      "|/iq/*/headers[@xmlns='" + XMLNS_SHIM + "']";
    return filter;
  }

  Tag* SHIM::tag() const
  {
    if( !m_headers.size() )
      return 0;

    Tag* t = new Tag( "headers" );
    t->setXmlns( XMLNS_SHIM );

    HeaderList::const_iterator it = m_headers.begin();
    for( ; it != m_headers.end(); ++it )
    {
      Tag* h = new Tag( t, "header" );
      h->addAttribute( "name", (*it).first );
      h->setCData( (*it).second );
    }
    return t;
  }

}
