/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "nonsaslauth.h"
#include "client.h"
#include "error.h"
#include "sha.h"

#include <string>

namespace gloox
{

  // ---- NonSaslAuth::Query ----
  NonSaslAuth::Query::Query( const std::string& user )
    : StanzaExtension( ExtNonSaslAuth ), m_user( user ), m_digest( true )
  {
  }

  NonSaslAuth::Query::Query( const Tag* tag )
    : StanzaExtension( ExtNonSaslAuth )
  {
    if( !tag || tag->name() != "query" || tag->xmlns() != XMLNS_AUTH )
      return;

    m_digest = tag->hasChild( "digest" );
  }

  NonSaslAuth::Query* NonSaslAuth::Query::newInstance( const std::string& user,
                                                       const std::string& sid,
                                                       const std::string& pwd,
                                                       const std::string& resource ) const
  {
    Query* q = new Query( user );
    if( m_digest && !sid.empty() )
    {
      SHA sha;
      sha.feed( sid );
      sha.feed( pwd );
      q->m_pwd = sha.hex();
    }
    else
      q->m_pwd = pwd;

    q->m_resource = resource;
    q->m_digest = m_digest;
    return q;
  }

  const std::string& NonSaslAuth::Query::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_AUTH + "']";
    return filter;
  }

  Tag* NonSaslAuth::Query::tag() const
  {
    if( m_user.empty() )
      return 0;

    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_AUTH );
    new Tag( t, "username", m_user );

    if( !m_pwd.empty() && !m_resource.empty() )
    {
      new Tag( t, m_digest ? "digest" : "password", m_pwd );
      new Tag( t, "resource", m_resource );
    }

    return t;
  }
  // ---- ~NonSaslAuth::Query ----

  // ---- NonSaslAuth ----
  NonSaslAuth::NonSaslAuth( Client* parent )
    : m_parent( parent )
  {
    if( m_parent )
    {
      m_parent->registerStanzaExtension( new Query() );
      m_parent->registerIqHandler( this, ExtNonSaslAuth );
    }
  }

  NonSaslAuth::~NonSaslAuth()
  {
    if( m_parent )
    {
      m_parent->removeStanzaExtension( ExtNonSaslAuth );
      m_parent->removeIqHandler( this, ExtNonSaslAuth );
      m_parent->removeIDHandler( this );
    }
  }

  void NonSaslAuth::doAuth( const std::string& sid )
  {
    m_sid = sid;
    const std::string& id = m_parent->getID();

    IQ iq( IQ::Get, m_parent->jid().server(), id );
    iq.addExtension( new Query( m_parent->username() ) );
    m_parent->send( iq, this, TrackRequestAuthFields );
  }

  void NonSaslAuth::handleIqID( const IQ& iq, int context )
  {
    switch( iq.subtype() )
    {
      case IQ::Error:
      {
        const Error* e = iq.error();
        if( e )
        {
          switch( e->error() )
          {
            case StanzaErrorConflict:
              m_parent->setAuthFailure( NonSaslConflict );
              break;
            case StanzaErrorNotAcceptable:
              m_parent->setAuthFailure( NonSaslNotAcceptable );
              break;
            case StanzaErrorNotAuthorized:
              m_parent->setAuthFailure( NonSaslNotAuthorized );
              break;
            default:
              break;
          }
        }
        m_parent->setAuthed( false );
        m_parent->disconnect( ConnAuthenticationFailed );
        break;
      }
      case IQ::Result:
        switch( context )
        {
          case TrackRequestAuthFields:
          {
            const Query* q = iq.findExtension<Query>( ExtNonSaslAuth );
            if( !q )
              return;

            const std::string& id = m_parent->getID();

            IQ re( IQ::Set, JID(), id );
            re.addExtension( q->newInstance( m_parent->username(), m_sid,
                                             m_parent->password(),
                                             m_parent->jid().resource() ) );
            m_parent->send( re, this, TrackSendAuth );
            break;
          }
          case TrackSendAuth:
            m_parent->setAuthed( true );
            m_parent->connected();
            break;
        }
        break;

      default:
        break;
    }
  }

}
