/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "component.h"

#include "disco.h"
#include "stanza.h"
#include "prep.h"
#include "sha.h"

#include <cstdlib>

namespace gloox
{

  Component::Component( const std::string& ns, const std::string& server,
                        const std::string& component, const std::string& password, int port )
    : ClientBase( ns, password, server, port )
  {
    m_jid.setServer( component );
    m_disco->setIdentity( "component", "generic" );
  }

  void Component::handleStartNode()
  {
    if( m_sid.empty() )
      return;

    notifyStreamEvent( StreamEventAuthentication );

    SHA sha;
    sha.feed( m_sid + m_password );
    sha.finalize();

    Tag* h = new Tag( "handshake", sha.hex() );
    send( h );
  }

  bool Component::handleNormalNode( Tag* tag )
  {
    if( tag->name() != "handshake" )
      return false;

    m_authed = true;
    notifyStreamEvent( StreamEventFinished );
    notifyOnConnect();

    return true;
  }

}
