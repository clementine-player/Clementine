/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "subscription.h"
#include "util.h"

namespace gloox
{

  static const char* msgTypeStringValues[] =
  {
    "subscribe", "subscribed", "unsubscribe", "unsubscribed"
  };

  static inline const std::string typeString( Subscription::S10nType type )
  {
    return util::lookup( type, msgTypeStringValues );
  }

  Subscription::Subscription( Tag* tag )
    : Stanza( tag ), m_subtype( Invalid ), m_stati( 0 )
  {
    if( !tag || tag->name() != "presence" )
      return;

    m_subtype = (S10nType)util::lookup( tag->findAttribute( TYPE ), msgTypeStringValues );

    const ConstTagList& c = tag->findTagList( "/presence/status" );
    ConstTagList::const_iterator it = c.begin();
    for( ; it != c.end(); ++it )
      setLang( &m_stati, m_status, (*it) );
  }

  Subscription::Subscription( S10nType type, const JID& to, const std::string& status,
                              const std::string& xmllang )
    : Stanza( to ), m_subtype( type ), m_stati( 0 )
  {
    setLang( &m_stati, m_status, status, xmllang );
  }

  Subscription::~Subscription()
  {
    delete m_stati;
  }

  Tag* Subscription::tag() const
  {
    if( m_subtype == Invalid )
      return 0;

    Tag* t = new Tag( "presence" );
    if( m_to )
      t->addAttribute( "to", m_to.full() );
    if( m_from )
      t->addAttribute( "from", m_from.full() );

    t->addAttribute( "type", typeString( m_subtype ) );

    getLangs( m_stati, m_status, "status", t );

    StanzaExtensionList::const_iterator it = m_extensionList.begin();
    for( ; it != m_extensionList.end(); ++it )
      t->addChild( (*it)->tag() );

    return t;
  }

}
