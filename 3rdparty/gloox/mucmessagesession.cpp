/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "mucmessagesession.h"
#include "clientbase.h"
#include "message.h"
#include "messagehandler.h"

namespace gloox
{

  MUCMessageSession::MUCMessageSession( ClientBase* parent, const JID& jid )
    : MessageSession( parent, jid, false, Message::Groupchat | Message::Chat
                                          | Message::Normal | Message::Error,
                      false )
  {
  }

  MUCMessageSession::~MUCMessageSession()
  {
  }

  void MUCMessageSession::handleMessage( Message& msg )
  {
    if( m_messageHandler )
      m_messageHandler->handleMessage( msg );
  }

  void MUCMessageSession::send( const std::string& message )
  {
    Message m( Message::Groupchat, m_target, message );

//     decorate( m );

    m_parent->send( m );
  }

  void MUCMessageSession::setSubject( const std::string& subject )
  {
    Message m( Message::Groupchat, m_target.bareJID(), EmptyString, subject );
    m_parent->send( m );
  }

}
