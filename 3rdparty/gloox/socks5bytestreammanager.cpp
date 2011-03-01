/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "bytestreamhandler.h"
#include "socks5bytestreammanager.h"
#include "socks5bytestreamserver.h"
#include "socks5bytestream.h"
#include "clientbase.h"
#include "disco.h"
#include "error.h"
#include "connectionbase.h"
#include "sha.h"
#include "util.h"

#include <cstdlib>

namespace gloox
{

  // ---- SOCKS5BytestreamManager::Query ----
  static const char* s5bModeValues[] =
  {
    "tcp", "udp"
  };

  static inline const char* modeString( SOCKS5BytestreamManager::S5BMode mode )
  {
    return s5bModeValues[mode];
  }

  SOCKS5BytestreamManager::Query::Query()
    : StanzaExtension( ExtS5BQuery ), m_type( TypeInvalid )
  {
  }

  SOCKS5BytestreamManager::Query::Query( const std::string& sid, S5BMode mode,
         const StreamHostList& hosts )
    : StanzaExtension( ExtS5BQuery ), m_sid( sid ), m_mode( mode ), m_hosts( hosts ), m_type( TypeSH )
  {
  }

  SOCKS5BytestreamManager::Query::Query( const JID& jid, const std::string& sid, bool activate )
    : StanzaExtension( ExtS5BQuery ), m_sid( sid ), m_jid( jid ), m_type( activate ? TypeA : TypeSHU )
  {
  }

  SOCKS5BytestreamManager::Query::Query( const Tag* tag )
    : StanzaExtension( ExtS5BQuery ), m_type( TypeInvalid )
  {
    if( !tag || tag->name() != "query" || tag->xmlns() != XMLNS_BYTESTREAMS
        /*|| !tag->hasAttribute( "sid" )*/ )
      return;

    m_sid = tag->findAttribute( "sid" );
    m_mode = static_cast<S5BMode>( util::deflookup( tag->findAttribute( "mode" ), s5bModeValues, S5BTCP ) );

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "streamhost" && (*it)->hasAttribute( "jid" )
            && (*it)->hasAttribute( "host" ) && (*it)->hasAttribute( "port" ) )
      {
        m_type = TypeSH;
        StreamHost sh;
        sh.jid = (*it)->findAttribute( "jid" );
        sh.host = (*it)->findAttribute( "host" );
        sh.port = atoi( (*it)->findAttribute( "port" ).c_str() );
        m_hosts.push_back( sh );
      }
      else if( (*it)->name() == "streamhost-used" )
      {
        m_type = TypeSHU;
        m_jid = (*it)->findAttribute( "jid" );
      }
      else if( (*it)->name() == "activate" )
      {
        m_type = TypeA;
        m_jid = (*it)->cdata();
      }
    }
  }

  SOCKS5BytestreamManager::Query::~Query()
  {
  }

  const std::string& SOCKS5BytestreamManager::Query::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_BYTESTREAMS + "']";
    return filter;
  }

  Tag* SOCKS5BytestreamManager::Query::tag() const
  {
    if( m_type == TypeInvalid /*|| m_sid.empty()*/ )
      return 0;

    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_BYTESTREAMS );
    t->addAttribute( "sid", m_sid );
    switch( m_type )
    {
      case TypeSH:
      {
        t->addAttribute( "mode", util::deflookup( m_mode, s5bModeValues, "tcp" ) );
        StreamHostList::const_iterator it = m_hosts.begin();
        for( ; it != m_hosts.end(); ++it )
        {
          Tag* s = new Tag( t, "streamhost" );
          s->addAttribute( "jid", (*it).jid.full() );
          s->addAttribute( "host", (*it).host );
          s->addAttribute( "port", (*it).port );
        }
        break;
      }
      case TypeSHU:
      {
        Tag* s = new Tag( t, "streamhost-used" );
        s->addAttribute( "jid", m_jid.full() );
        break;
      }
      case TypeA:
      {
        Tag* c = new Tag( t, "activate" );
        c->setCData( m_jid.full() );
        break;
      }
      default:
        break;
    }

    return t;
  }
  // ---- ~SOCKS5BytestreamManager::Query ----

  // ---- SOCKS5BytestreamManager ----
  SOCKS5BytestreamManager::SOCKS5BytestreamManager( ClientBase* parent, BytestreamHandler* s5bh )
    : m_parent( parent ), m_socks5BytestreamHandler( s5bh ), m_server( 0 )
  {
    if( m_parent )
    {
      m_parent->registerStanzaExtension( new Query() );
      m_parent->registerIqHandler( this, ExtS5BQuery );
    }
  }

  SOCKS5BytestreamManager::~SOCKS5BytestreamManager()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( this, ExtS5BQuery );
      m_parent->removeIDHandler( this );
    }

    util::clearMap( m_s5bMap );
  }

  void SOCKS5BytestreamManager::addStreamHost( const JID& jid, const std::string& host, int port )
  {
    StreamHost sh;
    sh.jid = jid;
    sh.host = host;
    sh.port = port;
    m_hosts.push_back( sh );
  }

  bool SOCKS5BytestreamManager::requestSOCKS5Bytestream( const JID& to, S5BMode mode,
                                                         const std::string& sid,
                                                         const JID& from )
  {
    if( !m_parent )
    {
      m_parent->logInstance().warn( LogAreaClassS5BManager,
                                    "No parent (ClientBase) set, cannot request bytestream." );
      return false;
    }

    if( m_hosts.empty() )
    {
      m_parent->logInstance().warn( LogAreaClassS5BManager,
                                    "No stream hosts set, cannot request bytestream." );
      return false;
    }

    const std::string& msid = sid.empty() ? m_parent->getID() : sid;
    const std::string& id = m_parent->getID();
    IQ iq( IQ::Set, to, id );
    iq.addExtension( new Query( msid, mode, m_hosts ) );
    if( from )
      iq.setFrom( from );

    if( m_server )
    {
      SHA sha;
      sha.feed( msid );
      if( from )
        sha.feed( from.full() );
      else
        sha.feed( m_parent->jid().full() );
      sha.feed( to.full() );
      m_server->registerHash( sha.hex() );
    }

    AsyncS5BItem asi;
    asi.sHosts = m_hosts;
    asi.id = id;
    asi.from = to;
    asi.to = from ? from : m_parent->jid();
    asi.incoming = false;
    m_asyncTrackMap[msid] = asi;

    m_trackMap[id] = msid;
    m_parent->send( iq, this, S5BOpenStream );

    return true;
  }

  void SOCKS5BytestreamManager::acknowledgeStreamHost( bool success, const JID& jid,
                                                       const std::string& sid )
  {
    AsyncTrackMap::const_iterator it = m_asyncTrackMap.find( sid );
    if( it == m_asyncTrackMap.end() || !m_parent )
      return;

    const AsyncS5BItem& item = (*it).second;

    IQ* iq = 0;

    if( item.incoming )
    {
      iq = new IQ( IQ::Result, item.from.full(), item.id );
      if( item.to )
        iq->setFrom( item.to );

      if( success )
        iq->addExtension( new Query( jid, sid, false ) );
      else
        iq->addExtension( new Error( StanzaErrorTypeCancel, StanzaErrorItemNotFound ) );

      m_parent->send( *iq );
    }
    else
    {
      if( success )
      {
        const std::string& id = m_parent->getID();
        iq = new IQ( IQ::Set, jid.full(), id );
        iq->addExtension( new Query( item.from, sid, true ) );

        m_trackMap[id] = sid;
        m_parent->send( *iq, this, S5BActivateStream );
      }
    }

    delete iq;
  }

  bool SOCKS5BytestreamManager::handleIq( const IQ& iq )
  {
    const Query* q = iq.findExtension<Query>( ExtS5BQuery );
    if( !q || !m_socks5BytestreamHandler
        || m_trackMap.find( iq.id() ) != m_trackMap.end() )
      return false;

    switch( iq.subtype() )
    {
      case IQ::Set:
      {
        const std::string& sid = q->sid();
// FIXME What is haveStream() good for?
        if( /*haveStream( iq.from() ) ||*/ sid.empty() || q->mode() == S5BUDP )
        {
          rejectSOCKS5Bytestream( iq.from(), iq.id(), StanzaErrorNotAcceptable );
          return true;
        }
        AsyncS5BItem asi;
        asi.sHosts = q->hosts();
        asi.id = iq.id();
        asi.from = iq.from();
        asi.to = iq.to();
        asi.incoming = true;
        m_asyncTrackMap[sid] = asi;
        m_socks5BytestreamHandler->handleIncomingBytestreamRequest( sid, iq.from() );
        break;
      }
      case IQ::Error:
        m_socks5BytestreamHandler->handleBytestreamError( iq, EmptyString );
        break;
      default:
        break;
    }

    return true;
  }

  const StreamHost* SOCKS5BytestreamManager::findProxy( const JID& from, const std::string& hostjid,
                                                        const std::string& sid )
  {
    AsyncTrackMap::const_iterator it = m_asyncTrackMap.find( sid );
    if( it == m_asyncTrackMap.end() )
      return 0;

    if( (*it).second.from == from )
    {
      StreamHostList::const_iterator it2 = (*it).second.sHosts.begin();
      for( ; it2 != (*it).second.sHosts.end(); ++it2 )
      {
        if( (*it2).jid == hostjid )
        {
          return &(*it2);
        }
      }
    }

    return 0;
  }

  bool SOCKS5BytestreamManager::haveStream( const JID& from )
  {
    S5BMap::const_iterator it = m_s5bMap.begin();
    for( ; it != m_s5bMap.end(); ++it )
    {
      if( (*it).second && (*it).second->target() == from )
        return true;
    }
    return false;
  }

  void SOCKS5BytestreamManager::acceptSOCKS5Bytestream( const std::string& sid )
  {
    AsyncTrackMap::iterator it = m_asyncTrackMap.find( sid );
    if( it == m_asyncTrackMap.end() || !m_socks5BytestreamHandler )
      return;

    SOCKS5Bytestream* s5b = new SOCKS5Bytestream( this, m_parent->connectionImpl()->newInstance(),
                                                  m_parent->logInstance(),
                                                  (*it).second.from, (*it).second.to, sid );
    s5b->setStreamHosts( (*it).second.sHosts );
    m_s5bMap[sid] = s5b;
    m_socks5BytestreamHandler->handleIncomingBytestream( s5b );
  }

  void SOCKS5BytestreamManager::rejectSOCKS5Bytestream( const std::string& sid, StanzaError reason )
  {
    AsyncTrackMap::iterator it = m_asyncTrackMap.find( sid );
    if( it != m_asyncTrackMap.end() )
    {
      rejectSOCKS5Bytestream( (*it).second.from, (*it).second.id, reason );
      m_asyncTrackMap.erase( it );
    }
  }

  void SOCKS5BytestreamManager::rejectSOCKS5Bytestream( const JID& from,
                                                        const std::string& id,
                                                        StanzaError reason )
  {
    IQ iq( IQ::Error, from, id );

    switch( reason )
    {
      case StanzaErrorForbidden:
      case StanzaErrorNotAcceptable:
      {
        iq.addExtension( new Error( StanzaErrorTypeAuth, reason ) );
        break;
      }
      case StanzaErrorFeatureNotImplemented:
      case StanzaErrorNotAllowed:
      default:
      {
        iq.addExtension( new Error( StanzaErrorTypeCancel, reason ) );
        break;
      }
    }

    m_parent->send( iq );
  }

  void SOCKS5BytestreamManager::handleIqID( const IQ& iq, int context )
  {
    StringMap::iterator it = m_trackMap.find( iq.id() );
    if( it == m_trackMap.end() )
      return;

    switch( context )
    {
      case S5BOpenStream:
      {
        switch( iq.subtype() )
        {
          case IQ::Result:
          {
            const Query* q = iq.findExtension<Query>( ExtS5BQuery );
            if( q && m_socks5BytestreamHandler )
            {
              const std::string& proxy = q->jid().full();
              const StreamHost* sh = findProxy( iq.from(), proxy, (*it).second );
              if( sh )
              {
                SOCKS5Bytestream* s5b = 0;
                bool selfProxy = ( proxy == m_parent->jid().full() && m_server );
                if( selfProxy )
                {
                  SHA sha;
                  sha.feed( (*it).second );
                  sha.feed( iq.to().full() );
                  sha.feed( iq.from().full() );
                  s5b = new SOCKS5Bytestream( this, m_server->getConnection( sha.hex() ),
                                              m_parent->logInstance(),
                                                  iq.to(), iq.from(),
                                                      (*it).second );
                }
                else
                {
                  s5b = new SOCKS5Bytestream( this, m_parent->connectionImpl()->newInstance(),
                                              m_parent->logInstance(),
                                                  iq.to(), iq.from(),
                                                      (*it).second );
                  s5b->setStreamHosts( StreamHostList( 1, *sh ) );
                }
                m_s5bMap[(*it).second] = s5b;
                m_socks5BytestreamHandler->handleOutgoingBytestream( s5b );
                if( selfProxy )
                  s5b->activate();
              }
            }
            break;
          }
          case IQ::Error:
            m_socks5BytestreamHandler->handleBytestreamError( iq, (*it).second );
            break;
          default:
            break;
        }
        break;
      }
      case S5BActivateStream:
      {
        switch( iq.subtype() )
        {
          case IQ::Result:
          {
            S5BMap::const_iterator it5 = m_s5bMap.find( (*it).second );
            if( it5 != m_s5bMap.end() )
              (*it5).second->activate();
            break;
          }
          case IQ::Error:
            m_socks5BytestreamHandler->handleBytestreamError( iq, (*it).second );
            break;
          default:
            break;
        }
        break;
      }
      default:
        break;
    }
    m_trackMap.erase( it );
  }

  bool SOCKS5BytestreamManager::dispose( SOCKS5Bytestream* s5b )
  {
    S5BMap::iterator it = m_s5bMap.find( s5b->sid() );
    if( it != m_s5bMap.end() )
    {
      delete s5b;
      m_s5bMap.erase( it );
      return true;
    }

    return false;
  }

}
