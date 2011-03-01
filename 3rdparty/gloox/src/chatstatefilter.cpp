/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "chatstatefilter.h"
#include "chatstatehandler.h"
#include "messageeventhandler.h"
#include "messagesession.h"
#include "message.h"
#include "chatstate.h"

namespace gloox
{

  ChatStateFilter::ChatStateFilter( MessageSession* parent )
    : MessageFilter( parent ), m_chatStateHandler( 0 ), m_lastSent( ChatStateGone ),
      m_enableChatStates( true )
  {
  }

  ChatStateFilter::~ChatStateFilter()
  {
  }

  void ChatStateFilter::filter( Message& msg )
  {
    if( m_enableChatStates && m_chatStateHandler )
    {
      const ChatState* state = msg.findExtension<ChatState>( ExtChatState );

      m_enableChatStates = state && state->state() != ChatStateInvalid;
      if( m_enableChatStates && msg.body().empty() )
        m_chatStateHandler->handleChatState( msg.from(), state->state() );
    }
  }

  void ChatStateFilter::setChatState( ChatStateType state )
  {
    if( !m_enableChatStates || state == m_lastSent || state == ChatStateInvalid )
      return;

    Message m( Message::Chat, m_parent->target() );
    m.addExtension( new ChatState( state ) );

    m_lastSent = state;

    send( m );
  }

  void ChatStateFilter::decorate( Message& msg )
  {
    if( m_enableChatStates )
      msg.addExtension( new ChatState( ChatStateActive ) );
  }

}
