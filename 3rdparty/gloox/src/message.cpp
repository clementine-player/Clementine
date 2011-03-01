/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "util.h"
#include "message.h"

namespace gloox
{

  static const char* msgTypeStringValues[] =
  {
    "chat", "error", "groupchat", "headline", "normal"
  };

  static inline const std::string typeString( Message::MessageType type )
  {
    return util::lookup2( type, msgTypeStringValues );
  }

  Message::Message( Tag* tag )
    : Stanza( tag ), m_subtype( Invalid ), m_bodies( 0 ), m_subjects( 0 )
  {
    if( !tag || tag->name() != "message" )
      return;

    const std::string& typestring = tag->findAttribute( TYPE );
    if( typestring.empty() )
      m_subtype = Normal;
    else
      m_subtype = (MessageType)util::lookup2( typestring, msgTypeStringValues );

    const TagList& c = tag->children();
    TagList::const_iterator it = c.begin();
    for( ; it != c.end(); ++it )
    {
      if( (*it)->name() == "body" )
        setLang( &m_bodies, m_body, (*it) );
      else if( (*it)->name() == "subject" )
        setLang( &m_subjects, m_subject, (*it) );
      else if( (*it)->name() == "thread" )
        m_thread = (*it)->cdata();
    }
  }

  Message::Message( MessageType type, const JID& to,
                    const std::string& body, const std::string& subject,
                    const std::string& thread, const std::string& xmllang )
    : Stanza( to ), m_subtype( type ), m_bodies( 0 ), m_subjects( 0 ), m_thread( thread )
  {
    setLang( &m_bodies, m_body, body, xmllang );
    setLang( &m_subjects, m_subject, subject, xmllang );
  }

  Message::~Message()
  {
    delete m_bodies;
    delete m_subjects;
  }

  Tag* Message::tag() const
  {
    if( m_subtype == Invalid )
      return 0;

    Tag* t = new Tag( "message" );
    if( m_to )
      t->addAttribute( "to", m_to.full() );
    if( m_from )
      t->addAttribute( "from", m_from.full() );
    if( !m_id.empty() )
      t->addAttribute( "id", m_id );
    t->addAttribute( TYPE, typeString( m_subtype ) );

    getLangs( m_bodies, m_body, "body", t );
    getLangs( m_subjects, m_subject, "subject", t );

    if( !m_thread.empty() )
      new Tag( t, "thread", m_thread );

    StanzaExtensionList::const_iterator it = m_extensionList.begin();
    for( ; it != m_extensionList.end(); ++it )
      t->addChild( (*it)->tag() );

    return t;
  }

}
