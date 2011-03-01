/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "iq.h"
#include "util.h"

namespace gloox
{

  static const char * iqTypeStringValues[] =
  {
    "get", "set", "result", "error"
  };

  static inline const char* typeString( IQ::IqType type )
  {
    return iqTypeStringValues[type];
  }

  IQ::IQ( Tag* tag )
    : Stanza( tag ), m_subtype( Invalid )
  {
    if( !tag || tag->name() != "iq" )
      return;

    m_subtype = (IQ::IqType)util::lookup( tag->findAttribute( TYPE ), iqTypeStringValues );
  }

  IQ::IQ( IqType type, const JID& to, const std::string& id )
    : Stanza( to ), m_subtype( type )
  {
    m_id = id;
  }

  IQ::~IQ()
  {
  }

  Tag* IQ::tag() const
  {
    if( m_subtype == Invalid )
      return 0;

    Tag* t = new Tag( "iq" );
    if( m_to )
      t->addAttribute( "to", m_to.full() );
    if( m_from )
      t->addAttribute( "from", m_from.full() );
    if( !m_id.empty() )
      t->addAttribute( "id", m_id );
    t->addAttribute( TYPE, typeString( m_subtype ) );

    StanzaExtensionList::const_iterator it = m_extensionList.begin();
    for( ; it != m_extensionList.end(); ++it )
      t->addChild( (*it)->tag() );

    return t;
  }

}
