/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "oob.h"
#include "tag.h"

namespace gloox
{

  OOB::OOB( const std::string& url, const std::string& description, bool iqext )
    : StanzaExtension( ExtOOB ), m_url( url ), m_desc( description ), m_iqext( iqext ),
      m_valid( true )
  {
    if( m_url.empty() )
      m_valid = false;
  }

  OOB::OOB( const Tag* tag )
    : StanzaExtension( ExtOOB ), m_iqext( false ), m_valid( false )
  {
    if( tag && ( ( tag->name() == "x" && tag->hasAttribute( XMLNS, XMLNS_X_OOB ) ) ||
        ( tag && tag->name() == "query" && tag->hasAttribute( XMLNS, XMLNS_IQ_OOB ) ) ) )
    {
      if( tag->name() == "query" )
        m_iqext = true;
    }
    else
      return;

    if( tag->hasChild( "url" ) )
    {
      m_valid = true;
      m_url = tag->findChild( "url" )->cdata();
    }
    if( tag->hasChild( "desc" ) )
      m_desc = tag->findChild( "desc" )->cdata();
  }

  OOB::~OOB()
  {
  }

  const std::string& OOB::filterString() const
  {
    static const std::string filter =
           "/presence/x[@xmlns='" + XMLNS_X_OOB + "']"
           "|/message/x[@xmlns='" + XMLNS_X_OOB + "']"
           "|/iq/query[@xmlns='" + XMLNS_IQ_OOB + "']";
    return filter;
  }

  Tag* OOB::tag() const
  {
    if( !m_valid )
      return 0;

    Tag* t = 0;

    if( m_iqext )
      t = new Tag( "query", XMLNS, XMLNS_IQ_OOB );
    else
      t = new Tag( "x", XMLNS, XMLNS_X_OOB );

    new Tag( t, "url", m_url );
    if( !m_desc.empty() )
      new Tag( t, "desc", m_desc );

    return t;
  }

}
