/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "lastactivity.h"
#include "disco.h"
#include "discohandler.h"
#include "clientbase.h"
#include "error.h"
#include "lastactivityhandler.h"

#include <cstdlib>

namespace gloox
{

  // ---- LastActivity::Query ----
  LastActivity::Query::Query( const Tag* tag )
    : StanzaExtension( ExtLastActivity ), m_seconds( -1 )
  {
    if( !tag || tag->name() != "query" || tag->xmlns() != XMLNS_LAST )
      return;

    if( tag->hasAttribute( "seconds" ) )
      m_seconds = atoi( tag->findAttribute( "seconds" ).c_str() );

    m_status = tag->cdata();
  }

  LastActivity::Query::Query( const std::string& _status, long _seconds )
    : StanzaExtension( ExtLastActivity ), m_seconds( _seconds ),
      m_status( _status )
  {
  }

  LastActivity::Query::~Query()
  {
  }

  const std::string& LastActivity::Query::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_LAST + "']"
        "|/presence/query[@xmlns='" + XMLNS_LAST + "']";
    return filter;
  }

  Tag* LastActivity::Query::tag() const
  {
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_LAST );
    t->addAttribute( "seconds", m_seconds );
    t->setCData( m_status );
    return t;
  }
  // ---- ~LastActivity::Query ----

  // ---- LastActivity ----
  LastActivity::LastActivity( ClientBase* parent )
    : m_lastActivityHandler( 0 ), m_parent( parent ),
      m_active( time ( 0 ) )
  {
    if( m_parent )
    {
      m_parent->registerStanzaExtension( new Query() );
      m_parent->registerIqHandler( this, ExtLastActivity );
      m_parent->disco()->addFeature( XMLNS_LAST );
    }
  }

  LastActivity::~LastActivity()
  {
    if( m_parent )
    {
      m_parent->disco()->removeFeature( XMLNS_LAST );
      m_parent->removeIqHandler( this, ExtLastActivity );
      m_parent->removeIDHandler( this );
    }
  }

  void LastActivity::query( const JID& jid )
  {
    IQ iq( IQ::Get, jid, m_parent->getID() );
    iq.addExtension( new Query() );
    m_parent->send( iq, this, 0 );
  }

  bool LastActivity::handleIq( const IQ& iq )
  {
    const Query* q = iq.findExtension<Query>( ExtLastActivity );
    if( !q || iq.subtype() != IQ::Get )
      return false;

    IQ re( IQ::Result, iq.from(), iq.id() );
    re.addExtension( new Query( EmptyString, (long)( time( 0 ) - m_active ) ) );
    m_parent->send( re );

    return true;
  }

  void LastActivity::handleIqID( const IQ& iq, int /*context*/ )
  {
    if( !m_lastActivityHandler )
      return;

    if( iq.subtype() == IQ::Result )
    {
      const Query* q = iq.findExtension<Query>( ExtLastActivity );
      if( !q || q->seconds() < 0 )
        return;

      m_lastActivityHandler->handleLastActivityResult( iq.from(), q->seconds(), q->status() );
    }
    else if( iq.subtype() == IQ::Error && iq.error() )
      m_lastActivityHandler->handleLastActivityError( iq.from(), iq.error()->error() );
  }

  void LastActivity::resetIdleTimer()
  {
    m_active = time( 0 );
  }

}
