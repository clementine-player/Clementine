/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "stanza.h"
#include "error.h"
#include "jid.h"
#include "util.h"
#include "stanzaextension.h"
#include "stanzaextensionfactory.h"

#include <cstdlib>

namespace gloox
{

  Stanza::Stanza( const JID& to )
    : m_xmllang( "default" ), m_to( to )
  {
  }

  Stanza::Stanza( Tag* tag )
    : m_xmllang( "default" )
  {
    if( !tag )
      return;

    m_from.setJID( tag->findAttribute( "from" ) );
    m_to.setJID( tag->findAttribute( "to" ) );
    m_id = tag->findAttribute( "id" );
  }

  Stanza::~Stanza()
  {
    removeExtensions();
  }

  const Error* Stanza::error() const
  {
    return findExtension<Error>( ExtError );
  }

  void Stanza::addExtension( const StanzaExtension* se )
  {
    m_extensionList.push_back( se );
  }

  const StanzaExtension* Stanza::findExtension( int type ) const
  {
    StanzaExtensionList::const_iterator it = m_extensionList.begin();
    for( ; it != m_extensionList.end() && (*it)->extensionType() != type; ++it ) ;
    return it != m_extensionList.end() ? (*it) : 0;
  }

  void Stanza::removeExtensions()
  {
    util::clearList( m_extensionList );
  }

  void Stanza::setLang( StringMap** map,
                        std::string& defaultLang,
                        const Tag* tag )
  {
    const std::string& lang = tag ? tag->findAttribute( "xml:lang" ) : EmptyString;
    setLang( map, defaultLang, tag ? tag->cdata() : EmptyString, lang );
  }

  void Stanza::setLang( StringMap** map,
                        std::string& defaultLang,
                        const std::string& data,
                        const std::string& xmllang )
  {
    if( data.empty() )
      return;

    if( xmllang.empty() )
      defaultLang = data;
    else
    {
      if( !*map )
        *map = new StringMap();
      (**map)[xmllang] = data;
    }
  }

  const std::string& Stanza::findLang( const StringMap* map,
                                       const std::string& defaultData,
                                       const std::string& lang )
  {
    if( map && lang != "default" )
    {
      StringMap::const_iterator it = map->find( lang );
      if( it != map->end() )
        return (*it).second;
    }
    return defaultData;
  }

  void Stanza::getLangs( const StringMap* map,
                         const std::string& defaultData,
                         const std::string& name,
                         Tag* tag )
  {
    if( !defaultData.empty() )
      new Tag( tag, name, defaultData );

    if( !map )
      return;

    StringMap::const_iterator it = map->begin();
    for( ; it != map->end(); ++it )
    {
      Tag* t = new Tag( tag, name, "xml:lang", (*it).first );
      t->setCData( (*it).second );
    }
  }

}
