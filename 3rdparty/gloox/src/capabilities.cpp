/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "capabilities.h"

#include "base64.h"
#include "disco.h"
#include "dataform.h"
#include "sha.h"
#include "tag.h"

namespace gloox
{

  Capabilities::Capabilities( Disco* disco )
    : StanzaExtension( ExtCaps ), m_disco( disco ), m_node( GLOOX_CAPS_NODE ),
      m_hash( "sha-1" ), m_valid( false )
  {
    if( m_disco )
      m_valid = true;
  }

  Capabilities::Capabilities( const Tag* tag )
    : StanzaExtension( ExtCaps ), m_disco( 0 ), m_valid( false )
  {
    if( !tag || tag->name() != "c" || !tag->hasAttribute( XMLNS, XMLNS_CAPS )
        || !tag->hasAttribute( "node" ) || !tag->hasAttribute( "ver" ) )
      return;

    m_node = tag->findAttribute( "node" );
    m_ver = tag->findAttribute( "ver" );
    m_hash = tag->findAttribute( "hash" );
    m_valid = true;
  }

  Capabilities::~Capabilities()
  {
    if( m_disco )
      m_disco->removeNodeHandlers( const_cast<Capabilities*>( this ) );
  }

  const std::string Capabilities::ver() const
  {
    if( !m_disco )
      return m_ver;

    SHA sha;
    sha.feed( generate( m_disco->identities(), m_disco->features( true ), m_disco->form() ) );
    const std::string& hash = Base64::encode64( sha.binary() );
    m_disco->removeNodeHandlers( const_cast<Capabilities*>( this ) );
    m_disco->registerNodeHandler( const_cast<Capabilities*>( this ), m_node + '#' + hash );
    return hash;
  }

  std::string Capabilities::generate( const Disco::IdentityList& il, const StringList& features, const DataForm* form )
  {
    StringList sl;
    Disco::IdentityList::const_iterator it = il.begin();
    for( ; it != il.end(); ++it )
    {
      std::string id = (*it)->category();
      id += '/';
      id += (*it)->type();
      id += '/';
      // FIXME add xml:lang caps here. see XEP-0115 Section 5
      id += '/';
      id += (*it)->name();
      sl.push_back( id );
    }
    sl.sort();

    std::string s;
    StringList::const_iterator it2 = sl.begin();
    for( ; it2 != sl.end(); ++it2 )
    {
      s += (*it2);
      s += '<';
    }

    StringList f = features;
    f.sort();
    it2 = f.begin();
    for( ; it2 != f.end(); ++it2 )
    {
      s += (*it2);
      s += '<';
    }

    if( form )
    {
      DataForm::FieldList::const_iterator it3 = form->fields().begin();
      typedef std::map<std::string, StringList> MapSSL;

      MapSSL m;
      for( ; it3 != form->fields().end(); ++it3 )
      {
        if( (*it3)->name() == "FORM_TYPE" )
        {
          s += (*it3)->value();
          s += '<';
        }
        else
          m.insert( std::make_pair( (*it3)->name(), (*it3)->values() ) );
      }

      MapSSL::iterator it4 = m.begin();
      for( ; it4 != m.end(); ++it4 )
      {
        s += it4->first;
        s += '<';
        it2 = it4->second.begin();
        for( ; it2 != it4->second.end(); ++it2 )
        {
          s += (*it2);
          s += '<';
        }
      }
    }
    return s;
  }

  std::string Capabilities::generate( const Disco::Info* info )
  {
    return info ? generate( info->identities(), info->features(), info->form() ) : EmptyString;
  }

  std::string Capabilities::generate( const Disco* disco )
  {
    return disco ? generate( disco->identities(), disco->features(), disco->form() ) : EmptyString;
  }

  const std::string& Capabilities::filterString() const
  {
    static const std::string filter = "/presence/c[@xmlns='" + XMLNS_CAPS + "']";
    return filter;
  }

  Tag* Capabilities::tag() const
  {
    if( !m_valid || m_node.empty() )
      return 0;

    Tag* t = new Tag( "c" );
    t->setXmlns( XMLNS_CAPS );
    t->addAttribute( "hash", m_hash );
    t->addAttribute( "node", m_node );
    t->addAttribute( "ver", ver() );
    return t;
  }

  StringList Capabilities::handleDiscoNodeFeatures( const JID&, const std::string& )
  {
    return m_disco->features();
  }

  Disco::IdentityList Capabilities::handleDiscoNodeIdentities( const JID&, const std::string& )
  {
    const Disco::IdentityList& il = m_disco->identities();
    Disco::IdentityList ret;
    Disco::IdentityList::const_iterator it = il.begin();
    for( ; it != il.end(); ++it )
    {
      ret.push_back( new Disco::Identity( *(*it) ) );
    }
    return ret;
  }

  Disco::ItemList Capabilities::handleDiscoNodeItems( const JID&, const JID&, const std::string& )
  {
    return Disco::ItemList();
  }

}
