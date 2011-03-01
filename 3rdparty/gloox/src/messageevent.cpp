/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "messageevent.h"
#include "tag.h"
#include "util.h"

namespace gloox
{

  /* chat state type values */
  static const char* eventValues [] = {
    "offline",
    "delivered",
    "displayed",
    "composing"
  };

  MessageEvent::MessageEvent( const Tag* tag )
    : StanzaExtension( ExtMessageEvent ), m_event( MessageEventCancel )
  {
    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    int event = 0;
    for( ; it != l.end(); ++it )
      event |= util::lookup2( (*it)->name(), eventValues );
    if( event )
      m_event = event;
  }

  const std::string& MessageEvent::filterString() const
  {
    static const std::string filter = "/message/x[@xmlns='" + XMLNS_X_EVENT + "']";
    return filter;
  }

  Tag* MessageEvent::tag() const
  {
    Tag* x = new Tag( "x", XMLNS, XMLNS_X_EVENT );

    if( m_event & MessageEventOffline )
      new Tag( x, "offline" );
    if( m_event & MessageEventDelivered )
      new Tag( x, "delivered" );
    if( m_event & MessageEventDisplayed )
      new Tag( x, "displayed" );
    if( m_event & MessageEventComposing )
      new Tag( x, "composing" );

    if( !m_id.empty() )
      new Tag( x, "id", m_id );

    return x;
  }

}
