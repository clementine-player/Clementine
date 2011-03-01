/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "uniquemucroom.h"
#include "clientbase.h"
#include "jid.h"
#include "sha.h"

namespace gloox
{

  // ---- UniqueMUCRoom::Unique ----
  UniqueMUCRoom::Unique::Unique( const Tag* tag )
    : StanzaExtension( ExtMUCUnique )
  {
    if( !tag || tag->name() != "unique" || tag->xmlns() != XMLNS_MUC_UNIQUE )
      return;

    m_name = tag->cdata();
  }

  const std::string& UniqueMUCRoom::Unique::filterString() const
  {
    static const std::string filter = "/iq/unique[@xmlns='" + XMLNS_MUC_UNIQUE + "']";
    return filter;
  }

  Tag* UniqueMUCRoom::Unique::tag() const
  {
    Tag* t = new Tag( "unique" );
    t->setXmlns( XMLNS_MUC_UNIQUE );
    if( !m_name.empty() )
      t->setCData( m_name );
    return t;
  }
  // ---- ~UniqueMUCRoom::Unique ----

  // ---- UniqueMUCRoom ----
  UniqueMUCRoom::UniqueMUCRoom( ClientBase* parent, const JID& nick, MUCRoomHandler* mrh )
    : InstantMUCRoom( parent, nick, mrh )
  {
    if( m_parent )
    {
      m_parent->registerStanzaExtension( new Unique() );
    }
  }

  UniqueMUCRoom::~UniqueMUCRoom()
  {
    if( m_parent )
    {
      m_parent->removeIDHandler( this );
//       m_parent->removeStanzaExtension( ExtMUCUnique ); // don't remove, other rooms might need it
    }
  }

  void UniqueMUCRoom::join()
  {
    if( !m_parent || m_joined )
      return;

    IQ iq( IQ::Get, m_nick.server() );
    iq.addExtension( new Unique() );
    m_parent->send( iq, this, RequestUniqueName );
  }

  void UniqueMUCRoom::handleIqID( const IQ& iq, int context )
  {
    switch( iq.subtype() )
    {
      case IQ::Result:
        if( context == RequestUniqueName )
        {
          const Unique* u = iq.findExtension<Unique>( ExtMUCUnique );
          if( u )
          {
            if( !u->name().empty() )
              setName( u->name() );
          }
        }
        break;
      case IQ::Error:
        if( context == RequestUniqueName )
        {
          SHA s;
          s.feed( m_parent->jid().full() );
          s.feed( m_parent->getID() );
          setName( s.hex() );
        }
        break;
      default:
        break;
    }

    MUCRoom::join();
  }

}
