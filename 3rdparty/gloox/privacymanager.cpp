/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "privacymanager.h"
#include "clientbase.h"
#include "error.h"

namespace gloox
{

  // ---- PrivacyManager::Query ----
  PrivacyManager::Query::Query( const Tag* tag )
    : StanzaExtension( ExtPrivacy )
  {
    if( !tag )
      return;

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      const std::string& name = (*it)->findAttribute( "name" );
      if( (*it)->name() == "default" )
        m_default = name;
      else if( (*it)->name() == "active" )
        m_active = name;
      else if( (*it)->name() == "list" )
      {
        m_names.push_back( name );

        const TagList& l = (*it)->children();
        TagList::const_iterator it_l = l.begin();
        for( ; it_l != l.end(); ++it_l )
        {
          PrivacyItem::ItemType type;
          PrivacyItem::ItemAction action;
          int packetType = 0;

          const std::string& t = (*it_l)->findAttribute( TYPE );
          if( t == "jid" )
            type = PrivacyItem::TypeJid;
          else if( t == "group" )
            type = PrivacyItem::TypeGroup;
          else if( t == "subscription" )
            type = PrivacyItem::TypeSubscription;
          else
            type = PrivacyItem::TypeUndefined;

          const std::string& a = (*it_l)->findAttribute( "action" );
          if( a == "allow" )
            action = PrivacyItem::ActionAllow;
          else if( a == "deny" )
            action = PrivacyItem::ActionDeny;
          else
            action = PrivacyItem::ActionAllow;

          const std::string& value = (*it_l)->findAttribute( "value" );

          const TagList& c = (*it_l)->children();
          TagList::const_iterator it_c = c.begin();
          for( ; it_c != c.end(); ++it_c )
          {
            if( (*it_c)->name() == "iq" )
              packetType |= PrivacyItem::PacketIq;
            else if( (*it_c)->name() == "presence-out" )
              packetType |= PrivacyItem::PacketPresenceOut;
            else if( (*it_c)->name() == "presence-in" )
              packetType |= PrivacyItem::PacketPresenceIn;
            else if( (*it_c)->name() == "message" )
              packetType |= PrivacyItem::PacketMessage;
          }

          PrivacyItem item( type, action, packetType, value );
          m_items.push_back( item );
        }
      }
    }
  }

  PrivacyManager::Query::Query( IdType context, const std::string& name,
                                const PrivacyListHandler::PrivacyList& list )
    : StanzaExtension( ExtPrivacy ), m_context( context ), m_items( list )
  {
    m_names.push_back( name );
  }

  PrivacyManager::Query::~Query()
  {
  }

  const std::string& PrivacyManager::Query::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_PRIVACY + "']";
    return filter;
  }

  Tag* PrivacyManager::Query::tag() const
  {
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_PRIVACY );

    std::string child;
    switch( m_context )
    {
      case PLRequestList:
      case PLRemove:
      case PLStore:
        child = "list";
        break;
      case PLDefault:
      case PLUnsetDefault:
        child = "default";
        break;
      case PLActivate:
      case PLUnsetActivate:
        child = "active";
        break;
      default:
      case PLRequestNames:
        return t;
        break;
    }
    Tag* c = new Tag( t, child );

    if( !m_names.empty() )
      c->addAttribute( "name", (*m_names.begin()) );

    int count = 0;
    PrivacyListHandler::PrivacyList::const_iterator it = m_items.begin();
    for( ; it != m_items.end(); ++it )
    {
      Tag* i = new Tag( c, "item" );

      switch( (*it).type() )
      {
        case PrivacyItem::TypeJid:
          i->addAttribute( TYPE, "jid" );
          break;
        case PrivacyItem::TypeGroup:
          i->addAttribute( TYPE, "group" );
          break;
        case PrivacyItem::TypeSubscription:
          i->addAttribute( TYPE, "subscription" );
          break;
        default:
          break;
      }

      switch( (*it).action() )
      {
        case PrivacyItem::ActionAllow:
          i->addAttribute( "action", "allow" );
          break;
        case PrivacyItem::ActionDeny:
          i->addAttribute( "action", "deny" );
          break;
      }

      int pType = (*it).packetType();
      if( pType != 15 )
      {
        if( pType & PrivacyItem::PacketMessage )
          new Tag( i, "message" );
        if( pType & PrivacyItem::PacketPresenceIn )
          new Tag( i, "presence-in" );
        if( pType & PrivacyItem::PacketPresenceOut )
          new Tag( i, "presence-out" );
        if( pType & PrivacyItem::PacketIq )
          new Tag( i, "iq" );
      }

      i->addAttribute( "value", (*it).value() );
      i->addAttribute( "order", ++count );
    }

    return t;
  }
  // ---- ~PrivacyManager::Query ----

  // ---- PrivacyManager ----
  PrivacyManager::PrivacyManager( ClientBase* parent )
    : m_parent( parent ), m_privacyListHandler( 0 )
  {
    if( m_parent )
    {
      m_parent->registerStanzaExtension( new Query() );
      m_parent->registerIqHandler( this, ExtPrivacy );
    }
  }

  PrivacyManager::~PrivacyManager()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( this, ExtPrivacy );
      m_parent->removeIDHandler( this );
    }
  }

  std::string PrivacyManager::operation( IdType context, const std::string& name )
  {
    const std::string& id = m_parent->getID();
    IQ::IqType iqType = IQ::Set;
    if( context == PLRequestNames || context == PLRequestList )
      iqType = IQ::Get;
    IQ iq( iqType, JID(), id );
    iq.addExtension( new Query( context, name ) );
    m_parent->send( iq, this, context );
    return id;
  }

  std::string PrivacyManager::store( const std::string& name, const PrivacyListHandler::PrivacyList& list )
  {
    if( list.empty() )
      return EmptyString;

    const std::string& id = m_parent->getID();

    IQ iq( IQ::Set, JID(), id );
    iq.addExtension( new Query( PLStore, name, list ) );
    m_parent->send( iq, this, PLStore );
    return id;
  }

  bool PrivacyManager::handleIq( const IQ& iq )
  {
    const Query* q = iq.findExtension<Query>( ExtPrivacy );
    if( iq.subtype() != IQ::Set || !m_privacyListHandler
        || !q || q->name().empty() )
      return false;

    m_privacyListHandler->handlePrivacyListChanged( q->name() );
    IQ re( IQ::Result, JID(), iq.id() );
    m_parent->send( re );
    return true;
  }

  void PrivacyManager::handleIqID( const IQ& iq, int context )
  {
    if( !m_privacyListHandler )
      return;

    switch( iq.subtype() )
    {
      case IQ::Result:
        switch( context )
        {
          case PLStore:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultStoreSuccess );
            break;
          case PLActivate:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultActivateSuccess );
            break;
          case PLDefault:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultDefaultSuccess );
            break;
          case PLRemove:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultRemoveSuccess );
            break;
          case PLRequestNames:
          {
            const Query* q = iq.findExtension<Query>( ExtPrivacy );
            if( !q )
              return;
            m_privacyListHandler->handlePrivacyListNames( q->def(), q->active(),
                                                          q->names() );
            break;
          }
          case PLRequestList:
          {
            const Query* q = iq.findExtension<Query>( ExtPrivacy );
            if( !q )
              return;
            m_privacyListHandler->handlePrivacyList( q->name(), q->items() );
            break;
          }
        }
        break;

      case IQ::Error:
      {
        switch( iq.error()->error() )
        {
          case StanzaErrorConflict:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultConflict );
            break;
          case StanzaErrorItemNotFound:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultItemNotFound );
            break;
          case StanzaErrorBadRequest:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultBadRequest );
            break;
          default:
            m_privacyListHandler->handlePrivacyListResult( iq.id(), ResultUnknownError );
            break;
        }
        break;
      }

      default:
        break;
    }
  }

}
