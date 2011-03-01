/*
  Copyright (c) 2008-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "softwareversion.h"
#include "tag.h"

namespace gloox
{

  SoftwareVersion::SoftwareVersion( const std::string& name,
                                           const std::string& version,
                                           const std::string& os )
    : StanzaExtension( ExtVersion ), m_name( name ), m_version( version ), m_os( os )
  {
  }

  SoftwareVersion::SoftwareVersion( const Tag* tag )
    : StanzaExtension( ExtVersion )
  {
    if( !tag )
      return;

    Tag* t = tag->findChild( "name" );
    if( t )
      m_name = t->cdata();

    t = tag->findChild( "version" );
    if( t )
      m_version = t->cdata();

    t = tag->findChild( "os" );
    if( t )
      m_os = t->cdata();
  }

  SoftwareVersion::~SoftwareVersion()
  {
  }

  const std::string& SoftwareVersion::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_VERSION + "']";
    return filter;
  }

  Tag* SoftwareVersion::tag() const
  {
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_VERSION );

    if( !m_name.empty() )
      new Tag( t, "name", m_name );

    if( !m_version.empty() )
      new Tag( t, "version", m_version );

    if( !m_os.empty() )
      new Tag( t, "os", m_os );

    return t;
  }

}
