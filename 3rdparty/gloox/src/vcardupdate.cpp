/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "vcardupdate.h"
#include "tag.h"

namespace gloox
{

  VCardUpdate::VCardUpdate()
    : StanzaExtension( ExtVCardUpdate ),
      m_notReady( true ), m_noImage( true ), m_valid( true )
  {
  }

  VCardUpdate::VCardUpdate( const std::string& hash )
    : StanzaExtension( ExtVCardUpdate ),
      m_hash( hash ), m_notReady( false ), m_noImage( false ), m_valid( true )
  {
    if( m_hash.empty() )
    {
      m_noImage = true;
      m_valid = false;
    }
  }

  VCardUpdate::VCardUpdate( const Tag* tag )
    : StanzaExtension( ExtVCardUpdate ),
      m_notReady( true ), m_noImage( true ), m_valid( false )
  {
    if( tag && tag->name() == "x" && tag->hasAttribute( XMLNS, XMLNS_X_VCARD_UPDATE ) )
    {
      m_valid = true;
      if( tag->hasChild( "photo" ) )
      {
        m_notReady = false;
        m_hash = tag->findChild( "photo" )->cdata();
        if( !m_hash.empty() )
          m_noImage = false;
      }
    }
  }

  VCardUpdate::~VCardUpdate()
  {
  }

  const std::string& VCardUpdate::filterString() const
  {
    static const std::string filter = "/presence/x[@xmlns='" + XMLNS_X_VCARD_UPDATE + "']";
    return filter;
  }

  Tag* VCardUpdate::tag() const
  {
    if( !m_valid )
      return 0;

    Tag* x = new Tag( "x", XMLNS, XMLNS_X_VCARD_UPDATE );
    if( !m_notReady )
    {
      Tag* p = new Tag( x, "photo" );
      if( !m_noImage )
        p->setCData( m_hash );
    }
    return x;
  }

}
