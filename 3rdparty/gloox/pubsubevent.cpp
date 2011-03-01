/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "pubsubevent.h"
#include "tag.h"
#include "util.h"

namespace gloox
{

  namespace PubSub
  {

    static const char* eventTypeValues[] = {
      "collection",
      "configuration",
      "delete",
      "items",
      "items",
      "purge",
      "subscription"
    };

    Event::ItemOperation::ItemOperation( const ItemOperation& right )
      : retract( right.retract ), item( right.item ),
        payload( right.payload ? right.payload->clone() : 0 )
    {
    }

    Event::Event( const Tag* event )
      : StanzaExtension( ExtPubSubEvent ), m_type( PubSub::EventUnknown ),
        m_subscriptionIDs( 0 ), m_config( 0 ), m_itemOperations( 0 ), m_subscription( false )
    {
      if( !event || event->name() != "event" )
        return;

      const TagList& events = event->children();
      TagList::const_iterator it = events.begin();
      const Tag* tag = 0;
      for( ; it != events.end(); ++it )
      {
        tag = (*it);
        PubSub::EventType type = (PubSub::EventType)util::lookup( tag->name(), eventTypeValues );

        switch( type )
        {
          case PubSub::EventCollection:
            tag = tag->findChild( "node" );
            if( tag )
            {
              m_node = tag->findAttribute( "id" );
              if( ( m_config = tag->findChild( "x" ) ) )
                m_config = m_config->clone();
            }
            break;

          case PubSub::EventConfigure:
          case PubSub::EventDelete:
          case PubSub::EventPurge:
            m_node = tag->findAttribute( "node" );
            if( type == PubSub::EventConfigure
                && ( m_config = tag->findChild( "x" ) ) )
              m_config = m_config->clone();
            break;

          case PubSub::EventItems:
          case PubSub::EventItemsRetract:
          {
            if( !m_itemOperations )
              m_itemOperations = new ItemOperationList();

            m_node = tag->findAttribute( "node" );
            const TagList& items = tag->children();
            TagList::const_iterator itt = items.begin();
            for( ; itt != items.end(); ++itt )
            {
              tag = (*itt);
              bool retract = false;
              if( tag->name() == "retract" )
              {
                retract = true;
                type = PubSub::EventItemsRetract;
              }
              ItemOperation* op = new ItemOperation( retract,
                                                     tag->findAttribute( "id" ),
                                                     tag->clone() );
              m_itemOperations->push_back( op );
            }
            break;
          }

          case EventSubscription:
          {
            m_node = tag->findAttribute( "node" );
            m_jid.setJID( tag->findAttribute( "jid" ) );
            m_subscription = tag->hasAttribute( "subscription", "subscribed" );
            break;
          }

          case PubSub::EventUnknown:
            if( type == PubSub::EventUnknown )
            {
              if( tag->name() != "headers" || m_subscriptionIDs != 0 )
              {
                m_valid = false;
                return;
              }

              m_subscriptionIDs = new StringList();

              const TagList& headers = tag->children();
              TagList::const_iterator ith = headers.begin();
              for( ; ith != headers.end(); ++ith )
              {
                const std::string& name = (*ith)->findAttribute( "name" );
                if( name == "pubsub#subid" )
                  m_subscriptionIDs->push_back( (*ith)->cdata() );
                else if( name == "pubsub#collection" )
                  m_collection = (*ith)->cdata();
              }
            }

          default:
            continue;
        }
        m_type = type;
      }

      m_valid = true;
    }

    Event::Event( const std::string& node, PubSub::EventType type )
     : StanzaExtension( ExtPubSubEvent ), m_type( type ),
        m_node( node ), m_subscriptionIDs( 0 ), m_config( 0 ),
        m_itemOperations( 0 )
    {
      if( type != PubSub::EventUnknown )
        m_valid = true;
    }

    Event::~Event()
    {
      delete m_subscriptionIDs;
      delete m_config;
      if( m_itemOperations )
      {
        ItemOperationList::iterator it = m_itemOperations->begin();
        for( ; it != m_itemOperations->end(); ++it )
        {
          delete (*it)->payload;
          delete (*it);
        }
        delete m_itemOperations;
      }
    }

    void Event::addItem( ItemOperation* op )
    {
      if( !m_itemOperations )
        m_itemOperations = new ItemOperationList();

      m_itemOperations->push_back( op );
    }

    const std::string& Event::filterString() const
    {
      static const std::string filter = "/message/event[@xmlns='" + XMLNS_PUBSUB_EVENT + "']";
      return filter;
    }

    Tag* Event::tag() const
    {
      if( !m_valid )
        return 0;

      Tag* event = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
      Tag* child = new Tag( event, util::lookup( m_type, eventTypeValues ) );

      Tag* item = 0;

      switch( m_type )
      {
        case PubSub::EventCollection:
        {
          item = new Tag( child, "node", "id", m_node );
          item->addChildCopy( m_config );
          break;
        }

        case PubSub::EventPurge:
        case PubSub::EventDelete:
        case PubSub::EventConfigure:
          child->addAttribute( "node", m_node );
          if( m_type == PubSub::EventConfigure )
            child->addChildCopy( m_config );
          break;

        case PubSub::EventItems:
        case PubSub::EventItemsRetract:
        {
          child->addAttribute( "node", m_node );
          if( m_itemOperations )
          {
//            Tag* item;
            ItemOperation* op;
            ItemOperationList::const_iterator itt = m_itemOperations->begin();
            for( ; itt != m_itemOperations->end(); ++itt )
            {
              op = (*itt);
//               item = new Tag( child, op->retract ? "retract" : "item", "id", op->item );
              if( op->payload )
                child->addChildCopy( op->payload );
            }
          }
          break;
        }

        case EventSubscription:
        {
          child->addAttribute( "node", m_node );
          child->addAttribute( "jid", m_jid.full() );
          child->addAttribute( "subscription", m_subscription ? "subscribed" : "none" );
          break;
        }

        default:
          delete event;
          return 0;
      }

      if( m_subscriptionIDs || !m_collection.empty() )
      {
        Tag* headers = new Tag( event, "headers", XMLNS, "http://jabber.org/protocol/shim" );
        StringList::const_iterator it = m_subscriptionIDs->begin();
        for( ; it != m_subscriptionIDs->end(); ++it )
        {
          (new Tag( headers, "header", "name", "pubsub#subid" ))->setCData( (*it) );
        }

        if( !m_collection.empty() )
          (new Tag( headers, "header", "name", "pubsub#collection" ) )
            ->setCData( m_collection );
      }

      return event;
    }

    StanzaExtension* Event::clone() const
    {
      Event* e = new Event( m_node, m_type );
      e->m_subscriptionIDs = m_subscriptionIDs ? new StringList( *m_subscriptionIDs ) : 0;
      e->m_config = m_config ? m_config->clone() : 0;
      if( m_itemOperations )
      {
        e->m_itemOperations = new ItemOperationList();
        ItemOperationList::const_iterator it = m_itemOperations->begin();
        for( ; it != m_itemOperations->end(); ++it )
          e->m_itemOperations->push_back( new ItemOperation( *(*it) ) );
      }
      else
        e->m_itemOperations = 0;

      e->m_collection = m_collection;
      return e;
    }

  }

}
