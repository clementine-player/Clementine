/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "delayeddelivery.h"

#include "tag.h"

namespace gloox
{

  DelayedDelivery::DelayedDelivery( const JID& from, const std::string stamp, const std::string& reason )
    : StanzaExtension( ExtDelay ), m_from( from ), m_stamp( stamp ), m_reason( reason ), m_valid( false )
  {
    if( !m_stamp.empty() )
      m_valid = true;
  }


  DelayedDelivery::DelayedDelivery( const Tag* tag )
    : StanzaExtension( ExtDelay ), m_valid( false )
  {
    if( !tag || !tag->hasAttribute( "stamp" ) )
      return;
    if( !( tag->name() == "x" && tag->hasAttribute( XMLNS, XMLNS_X_DELAY ) ) )
      if( !( tag->name() == "delay" && tag->hasAttribute( XMLNS, XMLNS_DELAY ) ) )
        return;

    m_reason = tag->cdata();
    m_stamp = tag->findAttribute( "stamp" );
    m_from = tag->findAttribute( "from" );
    m_valid = true;
  }

  DelayedDelivery::~DelayedDelivery()
  {
  }

  const std::string& DelayedDelivery::filterString() const
  {
    static const std::string filter =
           "/presence/delay[@xmlns='" + XMLNS_DELAY + "']"
           "|/message/delay[@xmlns='" + XMLNS_DELAY + "']"
           "|/presence/x[@xmlns='" + XMLNS_X_DELAY + "']"
           "|/message/x[@xmlns='" + XMLNS_X_DELAY + "']";
    return filter;
  }

  Tag* DelayedDelivery::tag() const
  {
    if( !m_valid )
      return 0;

    Tag* t = new Tag( "delay" );
    t->addAttribute( XMLNS, XMLNS_DELAY );
    if( m_from )
      t->addAttribute( "from", m_from.full() );
    if( !m_stamp.empty() )
      t->addAttribute( "stamp", m_stamp );
    if( !m_reason.empty() )
      t->setCData( m_reason );
    return t;
  }

}
