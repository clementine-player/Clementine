/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "rosteritem.h"
#include "rosteritemdata.h"
#include "util.h"

namespace gloox
{

  RosterItem::RosterItem( const std::string& jid, const std::string& name )
    : m_data( new RosterItemData( jid, name, StringList() ) )
  {
  }

  RosterItem::RosterItem( const RosterItemData& data )
    : m_data( new RosterItemData( data ) )
  {
  }

  RosterItem::~RosterItem()
  {
    delete m_data;
    util::clearMap( m_resources );
  }

  void RosterItem::setName( const std::string& name )
  {
    if( m_data )
      m_data->setName( name );
  }

  const std::string& RosterItem::name() const
  {
    if( m_data )
      return m_data->name();
    else
      return EmptyString;
  }

  const std::string& RosterItem::jid() const
  {
    if( m_data )
      return m_data->jid();
    else
      return EmptyString;
  }

  void RosterItem::setSubscription( const std::string& subscription, const std::string& ask )
  {
    if( m_data )
      m_data->setSubscription( subscription, ask );
  }

  SubscriptionType RosterItem::subscription() const
  {
    if( m_data )
      return m_data->subscription();
    else
      return S10nNone;
  }

  void RosterItem::setGroups( const StringList& groups )
  {
    if( m_data )
      m_data->setGroups( groups );
  }

  const StringList RosterItem::groups() const
  {
    if( m_data )
      return m_data->groups();
    else
      return StringList();
  }

  bool RosterItem::changed() const
  {
    if( m_data )
      return m_data->changed();
    else
      return false;
  }

  void RosterItem::setSynchronized()
  {
    if( m_data )
      m_data->setSynchronized();
  }

  void RosterItem::setPresence( const std::string& resource, Presence::PresenceType presence )
  {
    if( m_resources.find( resource ) == m_resources.end() )
      m_resources[resource] = new Resource( 0, EmptyString, presence );
    else
      m_resources[resource]->setStatus( presence );
  }

  void RosterItem::setStatus( const std::string& resource, const std::string& msg )
  {
    if( m_resources.find( resource ) == m_resources.end() )
      m_resources[resource] = new Resource( 0, msg, Presence::Unavailable );
    else
      m_resources[resource]->setMessage( msg );
  }

  void RosterItem::setPriority( const std::string& resource, int priority )
  {
    if( m_resources.find( resource ) == m_resources.end() )
      m_resources[resource] = new Resource( priority, EmptyString, Presence::Unavailable );
    else
      m_resources[resource]->setPriority( priority );
  }

  const Resource* RosterItem::highestResource() const
  {
    int highestPriority = -255;
    Resource* highestResource = 0;
    ResourceMap::const_iterator it = m_resources.begin();
    for( ; it != m_resources.end() ; ++it )
    {
      if( (*it).second->priority() > highestPriority )
      {
        highestPriority = (*it).second->priority();
        highestResource = (*it).second;
      }
    }
    return highestResource;
  }

  void RosterItem::setExtensions( const std::string& resource, const StanzaExtensionList& exts )
  {
    if( m_resources.find( resource ) == m_resources.end() )
      m_resources[resource] = new Resource( 0, EmptyString, Presence::Unavailable );

    m_resources[resource]->setExtensions( exts );
  }

  void RosterItem::removeResource( const std::string& resource )
  {
    ResourceMap::iterator it = m_resources.find( resource );
    if( it != m_resources.end() )
    {
      delete (*it).second;
      m_resources.erase( it );
    }
  }

  bool RosterItem::online() const
  {
    return !m_resources.empty();
  }

  const Resource* RosterItem::resource( const std::string& res ) const
  {
    ResourceMap::const_iterator it = m_resources.find( res );
    return it != m_resources.end() ? (*it).second : 0;
  }

  void RosterItem::setData( const RosterItemData& rid )
  {
    delete m_data;
    m_data = new RosterItemData( rid );
  }

}
