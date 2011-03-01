/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "search.h"

#include "clientbase.h"
#include "dataform.h"
#include "iq.h"

namespace gloox
{

  // Search::Query ----
  Search::Query::Query( DataForm* form )
    : StanzaExtension( ExtSearch ), m_form( form ), m_fields( 0 )
  {
  }

  Search::Query::Query( int fields, const SearchFieldStruct& values )
    : StanzaExtension( ExtSearch ), m_form( 0 ), m_fields( fields ), m_values( values )
  {
  }

  Search::Query::Query( const Tag* tag )
    : StanzaExtension( ExtSearch ), m_form( 0 ), m_fields( 0 )
  {
    if( !tag || tag->name() != "query" || tag->xmlns() != XMLNS_SEARCH )
      return;

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "instructions" )
      {
        m_instructions = (*it)->cdata();
      }
      else if( (*it)->name() == "item" )
      {
        m_srl.push_back( new SearchFieldStruct( (*it) ) );
      }
      else if( (*it)->name() == "first" )
        m_fields |= SearchFieldFirst;
      else if( (*it)->name() == "last" )
        m_fields |= SearchFieldLast;
      else if( (*it)->name() == "email" )
        m_fields |= SearchFieldEmail;
      else if( (*it)->name() == "nick" )
        m_fields |= SearchFieldNick;
      else if( !m_form && (*it)->name() == "x" && (*it)->xmlns() == XMLNS_X_DATA )
        m_form = new DataForm( (*it) );
    }
  }

  Search::Query::~Query()
  {
    delete m_form;
    SearchResultList::iterator it = m_srl.begin();
    for( ; it != m_srl.end(); ++it )
      delete (*it);
  }

  const std::string& Search::Query::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_SEARCH + "']";
    return filter;
  }

  Tag* Search::Query::tag() const
  {
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_SEARCH );
    if( m_form )
      t->addChild( m_form->tag() );
    else if( m_fields )
    {
      if( !m_instructions.empty() )
        new Tag( t, "instructions", m_instructions );
      if( m_fields & SearchFieldFirst )
        new Tag( t, "first", m_values.first() );
      if( m_fields & SearchFieldLast )
        new Tag( t, "last", m_values.last() );
      if( m_fields & SearchFieldNick )
        new Tag( t, "nick", m_values.nick() );
      if( m_fields & SearchFieldEmail )
        new Tag( t, "email", m_values.email() );
    }
    else if( !m_srl.empty() )
    {
      SearchResultList::const_iterator it = m_srl.begin();
      for( ; it != m_srl.end(); ++it )
      {
        t->addChild( (*it)->tag() );
      }
    }
    return t;
  }
  // ---- ~Search::Query ----

  // ---- Search ----
  Search::Search( ClientBase* parent )
    : m_parent( parent )
  {
    if( m_parent )
      m_parent->registerStanzaExtension( new Query() );
  }

  Search::~Search()
  {
    if( m_parent )
    {
      m_parent->removeIDHandler( this );
      m_parent->removeStanzaExtension( ExtRoster );
    }
  }

  void Search::fetchSearchFields( const JID& directory, SearchHandler* sh )
  {
    if( !m_parent || !directory || !sh )
      return;

    const std::string& id = m_parent->getID();
    IQ iq( IQ::Get, directory, id );
    iq.addExtension( new Query() );
    m_track[id] = sh;
    m_parent->send( iq, this, FetchSearchFields );
  }

  void Search::search( const JID& directory, DataForm* form, SearchHandler* sh )
  {
    if( !m_parent || !directory || !sh )
      return;

    const std::string& id = m_parent->getID();
    IQ iq( IQ::Set, directory, id );
    iq.addExtension( new Query( form ) );

    m_track[id] = sh;
    m_parent->send( iq, this, DoSearch );
  }

  void Search::search( const JID& directory, int fields, const SearchFieldStruct& values, SearchHandler* sh )
  {
    if( !m_parent || !directory || !sh )
      return;

    const std::string& id = m_parent->getID();

    IQ iq( IQ::Set, directory );
    iq.addExtension( new Query( fields, values ) );

    m_track[id] = sh;
    m_parent->send( iq, this, DoSearch );
  }

  void Search::handleIqID( const IQ& iq, int context )
  {
    TrackMap::iterator it = m_track.find( iq.id() );
    if( it != m_track.end() )
    {
      switch( iq.subtype() )
      {
        case IQ::Result:
        {
          const Query* q = iq.findExtension<Query>( ExtSearch );
          if( !q )
            return;

          switch( context )
          {
            case FetchSearchFields:
            {
              if( q->form() )
              {
                (*it).second->handleSearchFields( iq.from(), q->form() );
              }
              else
              {
                (*it).second->handleSearchFields( iq.from(), q->fields(), q->instructions() );
              }
              break;
            }
            case DoSearch:
            {
              if( q->form() )
              {
                (*it).second->handleSearchResult( iq.from(), q->form() );
              }
              else
              {
                (*it).second->handleSearchResult( iq.from(), q->result() );
              }
              break;
            }
          }
          break;
        }
        case IQ::Error:
          (*it).second->handleSearchError( iq.from(), iq.error() );
          break;

        default:
          break;
      }

      m_track.erase( it );
    }

    return;
  }

}
