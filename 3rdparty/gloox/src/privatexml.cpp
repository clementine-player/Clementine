/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "privatexml.h"
#include "clientbase.h"
#include "stanza.h"

namespace gloox
{

  // ---- PrivateXML::Query ----
  PrivateXML::Query::Query( const Tag* tag )
    : StanzaExtension( ExtPrivateXML ), m_privateXML( 0 )
  {
    if( !tag )
      return;

    if( tag->name() == "query" && tag->xmlns() == XMLNS_PRIVATE_XML )
    {
      if( tag->children().size() )
        m_privateXML = tag->children().front()->clone();
    }
    else
      m_privateXML = tag;
  }

  const std::string& PrivateXML::Query::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_PRIVATE_XML + "']";
    return filter;
  }

  Tag* PrivateXML::Query::tag() const
  {
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_PRIVATE_XML );
    if( m_privateXML )
      t->addChild( m_privateXML->clone() );
    return t;
  }
  // ---- ~PrivateXML::Query ----

  // ---- PrivateXML ----
  PrivateXML::PrivateXML( ClientBase* parent )
    : m_parent( parent )
  {
    if( !m_parent )
      return;

    m_parent->registerIqHandler( this, ExtPrivateXML );
    m_parent->registerStanzaExtension( new Query() );
  }

  PrivateXML::~PrivateXML()
  {
    if( !m_parent )
      return;

    m_parent->removeIqHandler( this, ExtPrivateXML );
    m_parent->removeIDHandler( this );
    m_parent->removeStanzaExtension( ExtPrivateXML );
  }

  std::string PrivateXML::requestXML( const std::string& tag, const std::string& xmlns,
                                      PrivateXMLHandler* pxh )
  {
    const std::string& id = m_parent->getID();

    IQ iq( IQ::Get, JID(), id );
    iq.addExtension( new Query( tag, xmlns ) );

    m_track[id] = pxh;
    m_parent->send( iq, this, RequestXml );

    return id;
  }

  std::string PrivateXML::storeXML( const Tag* tag, PrivateXMLHandler* pxh )
  {
    const std::string& id = m_parent->getID();

    IQ iq( IQ::Set, JID(), id );
    iq.addExtension( new Query( tag ) );

    m_track[id] = pxh;
    m_parent->send( iq, this, StoreXml );

    return id;
  }

  void PrivateXML::handleIqID( const IQ& iq, int context )
  {
    TrackMap::iterator t = m_track.find( iq.id() );
    if( t == m_track.end() )
      return;

    if( iq.subtype() == IQ::Result )
    {
      if( context == RequestXml )
      {
        const Query* q = iq.findExtension<Query>( ExtPrivateXML );
        if( q )
          (*t).second->handlePrivateXML( q->privateXML() );
      }
      else if( context == StoreXml )
        (*t).second->handlePrivateXMLResult( iq.id(), PrivateXMLHandler::PxmlStoreOk );
    }
    else if( iq.subtype() == IQ::Error )
    {
      if( context == RequestXml )
        (*t).second->handlePrivateXMLResult( iq.id(), PrivateXMLHandler::PxmlRequestError );
      else if( context == StoreXml )
        (*t).second->handlePrivateXMLResult( iq.id(), PrivateXMLHandler::PxmlStoreError );
    }

    m_track.erase( t );
  }

}
