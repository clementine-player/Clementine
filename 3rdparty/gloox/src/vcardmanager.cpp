/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "vcardmanager.h"
#include "vcardhandler.h"
#include "vcard.h"
#include "clientbase.h"
#include "disco.h"
#include "error.h"

namespace gloox
{

  VCardManager::VCardManager( ClientBase* parent )
    : m_parent( parent )
  {
    if( m_parent )
    {
      m_parent->registerIqHandler( this, ExtVCard );
      m_parent->disco()->addFeature( XMLNS_VCARD_TEMP );
      m_parent->registerStanzaExtension( new VCard() );
    }
  }

  VCardManager::~VCardManager()
  {
    if( m_parent )
    {
      m_parent->disco()->removeFeature( XMLNS_VCARD_TEMP );
      m_parent->removeIqHandler( this, ExtVCard );
      m_parent->removeIDHandler( this );
    }
  }

  void VCardManager::fetchVCard( const JID& jid, VCardHandler* vch )
  {
    if( !m_parent || !vch )
      return;

    TrackMap::const_iterator it = m_trackMap.find( jid.bare() );
    if( it != m_trackMap.end() )
      return;

    const std::string& id = m_parent->getID();
    IQ iq ( IQ::Get, jid, id );
    iq.addExtension( new VCard() );

    m_trackMap[id] = vch;
    m_parent->send( iq, this,VCardHandler::FetchVCard  );
  }

  void VCardManager::cancelVCardOperations( VCardHandler* vch )
  {
    TrackMap::iterator t;
    TrackMap::iterator it = m_trackMap.begin();
    while( it != m_trackMap.end() )
    {
      t = it;
      ++it;
      if( (*t).second == vch )
        m_trackMap.erase( t );
    }
  }

  void VCardManager::storeVCard( VCard* vcard, VCardHandler* vch )
  {
    if( !m_parent || !vch )
      return;

    const std::string& id = m_parent->getID();
    IQ iq( IQ::Set, JID(), id );
    iq.addExtension( vcard );

    m_trackMap[id] = vch;
    m_parent->send( iq, this, VCardHandler::StoreVCard );
  }

  void VCardManager::handleIqID( const IQ& iq, int context )
  {
    TrackMap::iterator it = m_trackMap.find( iq.id() );
    if( it != m_trackMap.end() )
    {
      switch( iq.subtype() )
      {
        case IQ::Result:
        {
          switch( context )
          {
            case VCardHandler::FetchVCard:
            {
              const VCard* v = iq.findExtension<VCard>( ExtVCard );
              (*it).second->handleVCard( iq.from(), v );
              break;
            }
            case VCardHandler::StoreVCard:
              (*it).second->handleVCardResult( VCardHandler::StoreVCard, iq.from() );
              break;
          }
        }
        break;
        case IQ::Error:
        {
          (*it).second->handleVCardResult( (VCardHandler::VCardContext)context,
                                           iq.from(),
                                           iq.error() ? iq.error()->error()
                                                       : StanzaErrorUndefined );
          break;
        }
        default:
          break;
      }

      m_trackMap.erase( it );
    }
  }

}
