/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "stanzaextensionfactory.h"

#include "gloox.h"
#include "util.h"
#include "stanza.h"
#include "stanzaextension.h"
#include "tag.h"

namespace gloox
{

  StanzaExtensionFactory::StanzaExtensionFactory()
  {
  }

  StanzaExtensionFactory::~StanzaExtensionFactory()
  {
    util::clearList( m_extensions );
  }

  void StanzaExtensionFactory::registerExtension( StanzaExtension* ext )
  {
    if( !ext )
      return;

    SEList::iterator it = m_extensions.begin();
    SEList::iterator it2;
    while( it != m_extensions.end() )
    {
      it2 = it++;
      if( ext->extensionType() == (*it2)->extensionType() )
      {
        delete (*it2);
        m_extensions.erase( it2 );
      }
    }
    m_extensions.push_back( ext );
  }

  bool StanzaExtensionFactory::removeExtension( int ext )
  {
    SEList::iterator it = m_extensions.begin();
    for( ; it != m_extensions.end(); ++it )
    {
      if( (*it)->extensionType() == ext )
      {
        delete (*it);
        m_extensions.erase( it );
        return true;
      }
    }
    return false;
  }

  void StanzaExtensionFactory::addExtensions( Stanza& stanza, Tag* tag )
  {
    ConstTagList::const_iterator it;
    SEList::const_iterator ite = m_extensions.begin();
    for( ; ite != m_extensions.end(); ++ite )
    {
      const ConstTagList& match = tag->findTagList( (*ite)->filterString() );
      it = match.begin();
      for( ; it != match.end(); ++it )
      {
        StanzaExtension* se = (*ite)->newInstance( (*it) );
        if( se )
          stanza.addExtension( se );
      }
    }
  }

}
