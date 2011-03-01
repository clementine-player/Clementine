/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "nickname.h"
#include "tag.h"

namespace gloox
{

  Nickname::Nickname( const Tag* tag )
    : StanzaExtension( ExtNickname )
  {
    if( tag )
      m_nick = tag->cdata();
  }

  const std::string& Nickname::filterString() const
  {
    static const std::string filter =
           "/presence/nick[@xmlns='" + XMLNS_NICKNAME + "']"
           "|/message/nick[@xmlns='" + XMLNS_NICKNAME + "']";
    return filter;
  }

  Tag* Nickname::tag() const
  {
    if( m_nick.empty() )
      return 0;

    Tag* n = new Tag( "nick", XMLNS, XMLNS_NICKNAME );
    n->setCData( m_nick );
    return n;
  }

}
