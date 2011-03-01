/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "messageeventfilter.h"
#include "messageeventhandler.h"
#include "messagesession.h"
#include "message.h"
#include "messageevent.h"
#include "error.h"

namespace gloox
{

  MessageEventFilter::MessageEventFilter( MessageSession* parent )
    : MessageFilter( parent ), m_messageEventHandler( 0 ), m_requestedEvents( 0 ),
      m_lastSent( MessageEventCancel ), m_disable( false )
  {
  }

  MessageEventFilter::~MessageEventFilter()
  {
  }

  void MessageEventFilter::filter( Message& msg )
  {
    if( m_disable || !m_messageEventHandler )
      return;

    if( msg.subtype() == Message::Error )
    {
      if( msg.error() && msg.error()->error() == StanzaErrorFeatureNotImplemented )
        m_disable = true;

      return;
    }

    const MessageEvent* me = msg.findExtension<MessageEvent>( ExtMessageEvent );
    if( !me )
    {
      m_requestedEvents = 0;
      m_lastID = EmptyString;
      return;
    }

    if( msg.body().empty() )
      m_messageEventHandler->handleMessageEvent( msg.from(), (MessageEventType)me->event() );
    else
    {
      m_lastID = msg.id();
      m_requestedEvents = 0;
      m_requestedEvents = me->event();
    }
  }

  void MessageEventFilter::raiseMessageEvent( MessageEventType event )
  {
    if( m_disable || ( !( m_requestedEvents & event ) && ( event != MessageEventCancel ) ) )
      return;

    switch( event )
    {
      case MessageEventOffline:
      case MessageEventDelivered:
      case MessageEventDisplayed:
        m_requestedEvents &= ~event;
        break;
      case MessageEventComposing:
        if( m_lastSent == MessageEventComposing )
          return;
        break;
      case MessageEventCancel:
      default:
        break;
    }

    m_lastSent = event;
    Message m( Message::Normal, m_parent->target() );
    m.addExtension( new MessageEvent( event, m_lastID ) );
    send( m );
  }

  void MessageEventFilter::decorate( Message& msg )
  {
    if( m_disable )
      return;

    msg.addExtension( new MessageEvent( MessageEventOffline | MessageEventDelivered |
                                        MessageEventDisplayed | MessageEventComposing ) );
    m_lastSent = MessageEventCancel;
  }

  void MessageEventFilter::registerMessageEventHandler( MessageEventHandler* meh )
  {
    m_messageEventHandler = meh;
  }

  void MessageEventFilter::removeMessageEventHandler()
  {
    m_messageEventHandler = 0;
  }

}
