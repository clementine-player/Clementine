/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "simanager.h"

#include "siprofilehandler.h"
#include "sihandler.h"
#include "clientbase.h"
#include "disco.h"
#include "error.h"

namespace gloox
{

  // ---- SIManager::SI ----
  SIManager::SI::SI( const Tag* tag )
    : StanzaExtension( ExtSI ), m_tag1( 0 ), m_tag2( 0 )
  {
    if( !tag || tag->name() != "si" || tag->xmlns() != XMLNS_SI )
      return;

    m_valid = true;

    m_id = tag->findAttribute( "id" );
    m_mimetype = tag->findAttribute( "mime-type" );
    m_profile = tag->findAttribute( "profile" );

    Tag* c = tag->findChild( "file", "xmlns", XMLNS_SI_FT );
    if ( c )
      m_tag1 = c->clone();
    c = tag->findChild( "feature", "xmlns", XMLNS_FEATURE_NEG );
    if( c )
      m_tag2 = c->clone();
  }

  SIManager::SI::SI( Tag* tag1, Tag* tag2, const std::string& id,
                     const std::string& mimetype, const std::string& profile )
    : StanzaExtension( ExtSI ), m_tag1( tag1 ), m_tag2( tag2 ),
      m_id( id ), m_mimetype( mimetype ), m_profile( profile )
  {
    m_valid = true;
  }

  SIManager::SI::~SI()
  {
    delete m_tag1;
    delete m_tag2;
  }

  const std::string& SIManager::SI::filterString() const
  {
    static const std::string filter = "/iq/si[@xmlns='" + XMLNS_SI + "']";
    return filter;
  }

  Tag* SIManager::SI::tag() const
  {
    if( !m_valid )
      return 0;

    Tag* t = new Tag( "si" );
    t->setXmlns( XMLNS_SI );
    if( !m_id.empty() )
      t->addAttribute( "id", m_id );
    if( !m_mimetype.empty() )
      t->addAttribute( "mime-type", m_mimetype.empty() ? "binary/octet-stream" : m_mimetype );
    if( !m_profile.empty() )
      t->addAttribute( "profile", m_profile );
    if( m_tag1 )
      t->addChildCopy( m_tag1 );
    if( m_tag2 )
      t->addChildCopy( m_tag2 );

    return t;
  }
  // ---- ~SIManager::SI ----

  // ---- SIManager ----
  SIManager::SIManager( ClientBase* parent, bool advertise )
    : m_parent( parent ), m_advertise( advertise )
  {
    if( m_parent )
    {
      m_parent->registerStanzaExtension( new SI() );
      m_parent->registerIqHandler( this, ExtSI );
      if( m_parent->disco() && m_advertise )
        m_parent->disco()->addFeature( XMLNS_SI );
    }
  }

  SIManager::~SIManager()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( this, ExtSI );
      m_parent->removeIDHandler( this );
      if( m_parent->disco() && m_advertise )
        m_parent->disco()->removeFeature( XMLNS_SI );
    }
  }

  const std::string SIManager::requestSI( SIHandler* sih, const JID& to, const std::string& profile,
                                          Tag* child1, Tag* child2, const std::string& mimetype,
                                          const JID& from, const std::string& sid )
  {
    if( !m_parent || !sih )
      return EmptyString;

    const std::string& id = m_parent->getID();
    const std::string& sidToUse = sid.empty() ? m_parent->getID() : sid;

    IQ iq( IQ::Set, to, id );
    iq.addExtension( new SI( child1, child2, sidToUse, mimetype, profile ) );
    if( from )
      iq.setFrom( from );

    TrackStruct t;
    t.sid = sidToUse;
    t.profile = profile;
    t.sih = sih;
    m_track[id] = t;
    m_parent->send( iq, this, OfferSI );

    return sidToUse;
  }

  void SIManager::acceptSI( const JID& to, const std::string& id, Tag* child1, Tag* child2, const JID& from )
  {
    IQ iq( IQ::Result, to, id );
    iq.addExtension( new SI( child1, child2 ) );
    if( from )
      iq.setFrom( from );

    m_parent->send( iq );
  }

  void SIManager::declineSI( const JID& to, const std::string& id, SIError reason, const std::string& text )
  {
    IQ iq( IQ::Error, to, id );
    Error* error;
    if( reason == NoValidStreams || reason == BadProfile )
    {
      Tag* appError = 0;
      if( reason == NoValidStreams )
        appError = new Tag( "no-valid-streams", XMLNS, XMLNS_SI );
      else if( reason == BadProfile )
        appError = new Tag( "bad-profile", XMLNS, XMLNS_SI );
      error = new Error( StanzaErrorTypeCancel, StanzaErrorBadRequest, appError );
    }
    else
    {
      error = new Error( StanzaErrorTypeCancel, StanzaErrorForbidden );
      if( !text.empty() )
        error->text( text );
    }

    iq.addExtension( error );
    m_parent->send( iq );
  }

  void SIManager::registerProfile( const std::string& profile, SIProfileHandler* sih )
  {
    if( !sih || profile.empty() )
      return;

    m_handlers[profile] = sih;

    if( m_parent && m_advertise && m_parent->disco() )
      m_parent->disco()->addFeature( profile );
  }

  void SIManager::removeProfile( const std::string& profile )
  {
    if( profile.empty() )
      return;

    m_handlers.erase( profile );

    if( m_parent && m_advertise && m_parent->disco() )
      m_parent->disco()->removeFeature( profile );
  }

  bool SIManager::handleIq( const IQ& iq )
  {
    TrackMap::iterator itt = m_track.find( iq.id() );
    if( itt != m_track.end() )
      return false;

    const SI* si = iq.findExtension<SI>( ExtSI );
    if( !si || si->profile().empty() )
      return false;

    HandlerMap::const_iterator it = m_handlers.find( si->profile() );
    if( it != m_handlers.end() && (*it).second )
    {
      (*it).second->handleSIRequest( iq.from(), iq.to(), iq.id(), *si );
      return true;
    }

    return false;
  }

  void SIManager::handleIqID( const IQ& iq, int context )
  {
    switch( iq.subtype() )
    {
      case IQ::Result:
        if( context == OfferSI )
        {
          TrackMap::iterator it = m_track.find( iq.id() );
          if( it != m_track.end() )
          {
            const SI* si = iq.findExtension<SI>( ExtSI );
            if( !si /*|| si->profile().empty()*/ )
              return;

//             Tag* si = iq.query();
//             Tag* ptag = 0;
//             Tag* fneg = 0;
//             if( si && si->name() == "si" && si->xmlns() == XMLNS_SI )
//             {
//               ptag = si->findChildWithAttrib( XMLNS, (*it).second.profile );
//               fneg = si->findChild( "feature", XMLNS, XMLNS_FEATURE_NEG );
//             }

            // FIXME: remove above commented code and
            // check corectness of last 3 params!
            (*it).second.sih->handleSIRequestResult( iq.from(), iq.to(), (*it).second.sid, *si );
            m_track.erase( it );
          }
        }
        break;
      case IQ::Error:
        if( context == OfferSI )
        {
          TrackMap::iterator it = m_track.find( iq.id() );
          if( it != m_track.end() )
          {
            (*it).second.sih->handleSIRequestError( iq, (*it).second.sid );
            m_track.erase( it );
          }
        }
        break;
      default:
        break;
    }
  }

}
