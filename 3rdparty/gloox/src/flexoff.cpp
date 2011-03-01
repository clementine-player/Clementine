/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "flexoff.h"
#include "dataform.h"
#include "disco.h"
#include "error.h"

#include <cstdlib>

namespace gloox
{

  // ---- FlexibleOffline::Offline ----
  FlexibleOffline::Offline::Offline( const Tag* /*tag*/ )
    : StanzaExtension( ExtFlexOffline )
  {
    // FIXME what to do here?
  }

  FlexibleOffline::Offline::Offline( int context, const StringList& msgs )
    : StanzaExtension( ExtFlexOffline ), m_context( context ), m_msgs( msgs )
  {
  }

  FlexibleOffline::Offline::~Offline()
  {
  }

  const std::string& FlexibleOffline::Offline::filterString() const
  {
    static const std::string filter = "/iq/offline[@xmlns='" + XMLNS_OFFLINE + "']";
    return filter;
  }

  Tag* FlexibleOffline::Offline::tag() const
  {
    Tag* t = new Tag( "offline" );
    t->setXmlns( XMLNS_OFFLINE );

    if( m_msgs.empty() )
      new Tag( t, m_context == FORequestMsgs ? "fetch" : "purge" );
    else
    {
      const std::string action = m_context == FORequestMsgs ? "view" : "remove";
      StringList::const_iterator it = m_msgs.begin();
      for( ; it != m_msgs.end(); ++it )
      {
        Tag* i = new Tag( t, "item", "action", action );
        i->addAttribute( "node", (*it) );
      }
    }
    return t;
  }
  // ---- ~FlexibleOffline::Offline ----

  // ---- FlexibleOffline ----
  FlexibleOffline::FlexibleOffline( ClientBase* parent )
    : m_parent( parent ), m_flexibleOfflineHandler( 0 )
  {
    if( m_parent )
      m_parent->registerStanzaExtension( new Offline() );
  }

  FlexibleOffline::~FlexibleOffline()
  {
    if( m_parent )
      m_parent->removeIDHandler( this );
  }

  void FlexibleOffline::checkSupport()
  {
    m_parent->disco()->getDiscoInfo( m_parent->jid().server(), EmptyString, this, FOCheckSupport );
  }

  void FlexibleOffline::getMsgCount()
  {
    m_parent->disco()->getDiscoInfo( m_parent->jid().server(), XMLNS_OFFLINE, this, FORequestNum );
  }

  void FlexibleOffline::fetchHeaders()
  {
    m_parent->disco()->getDiscoItems( m_parent->jid().server(), XMLNS_OFFLINE, this, FORequestHeaders );
  }

  void FlexibleOffline::messageOperation( int context, const StringList& msgs )
  {
    const std::string& id = m_parent->getID();
    IQ::IqType iqType = context == FORequestMsgs ? IQ::Get : IQ::Set;
    IQ iq( iqType, JID(), id );
    iq.addExtension( new Offline( context, msgs ) );
    m_parent->send( iq, this, context );
  }

  void FlexibleOffline::registerFlexibleOfflineHandler( FlexibleOfflineHandler* foh )
  {
    m_flexibleOfflineHandler = foh;
  }

  void FlexibleOffline::removeFlexibleOfflineHandler()
  {
    m_flexibleOfflineHandler = 0;
  }

  void FlexibleOffline::handleDiscoInfo( const JID& /*from*/, const Disco::Info& info, int context )
  {
    if( !m_flexibleOfflineHandler )
      return;

    switch( context )
    {
      case FOCheckSupport:
        m_flexibleOfflineHandler->handleFlexibleOfflineSupport( info.hasFeature( XMLNS_OFFLINE ) );
        break;

      case FORequestNum:
        int num = -1;
        if( info.form() && info.form()->hasField( "number_of_messages" ) )
          num = atoi( info.form()->field( "number_of_messages" )->value().c_str() );

        m_flexibleOfflineHandler->handleFlexibleOfflineMsgNum( num );
        break;
    }
  }

  void FlexibleOffline::handleDiscoItems( const JID& /*from*/, const Disco::Items& items, int context )
  {
    if( context == FORequestHeaders && m_flexibleOfflineHandler )
    {
      if( items.node() == XMLNS_OFFLINE )
        m_flexibleOfflineHandler->handleFlexibleOfflineMessageHeaders( items.items() );
    }
  }

  void FlexibleOffline::handleDiscoError( const JID& /*from*/, const Error* /*error*/, int /*context*/ )
  {
  }

  void FlexibleOffline::handleIqID( const IQ& iq, int context )
  {
    if( !m_flexibleOfflineHandler )
      return;

    switch( context )
    {
      case FORequestMsgs:
        switch( iq.subtype() )
        {
          case IQ::Result:
            m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrRequestSuccess );
            break;
          case IQ::Error:
            switch( iq.error()->error() )
            {
              case StanzaErrorForbidden:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrForbidden );
                break;
              case StanzaErrorItemNotFound:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrItemNotFound );
                break;
              default:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrUnknownError );
                break;
            }
            break;
          default:
            break;
        }
        break;
      case FORemoveMsgs:
        switch( iq.subtype() )
        {
          case IQ::Result:
            m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrRemoveSuccess );
            break;
          case IQ::Error:
            switch( iq.error()->error() )
            {
              case StanzaErrorForbidden:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrForbidden );
                break;
              case StanzaErrorItemNotFound:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrItemNotFound );
                break;
              default:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrUnknownError );
                break;
            }
            break;
          default:
            break;
        }
        break;
    }
  }

}
