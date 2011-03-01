/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "annotations.h"
#include "clientbase.h"


namespace gloox
{

  Annotations::Annotations( ClientBase* parent )
    : PrivateXML( parent ),
      m_annotationsHandler( 0 )
  {
  }

  Annotations::~Annotations()
  {
  }

  void Annotations::storeAnnotations( const AnnotationsList& aList )
  {
    Tag* s = new Tag( "storage", XMLNS, XMLNS_ANNOTATIONS );

    AnnotationsList::const_iterator it = aList.begin();
    for( ; it != aList.end(); ++it )
    {
      Tag* n = new Tag( s, "note", (*it).note );
      n->addAttribute( "jid", (*it).jid );
      n->addAttribute( "cdate", (*it).cdate );
      n->addAttribute( "mdate", (*it).mdate );
    }

    storeXML( s, this );
  }

  void Annotations::requestAnnotations()
  {
    requestXML( "storage", XMLNS_ANNOTATIONS, this );
  }

  void Annotations::handlePrivateXML( const Tag* xml )
  {
    if( !xml )
      return;

    AnnotationsList aList;
    const TagList& l = xml->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "note" )
      {
        const std::string& jid = (*it)->findAttribute( "jid" );
        const std::string& note = (*it)->cdata();

        if( !jid.empty() && !note.empty() )
        {
          const std::string& cdate = (*it)->findAttribute( "cdate" );
          const std::string& mdate = (*it)->findAttribute( "mdate" );
          AnnotationsListItem item;
          item.jid = jid;
          item.cdate = cdate;
          item.mdate = mdate;
          item.note = note;
          aList.push_back( item );
        }
      }
    }

    if( m_annotationsHandler )
      m_annotationsHandler->handleAnnotations( aList );
  }

  void Annotations::handlePrivateXMLResult( const std::string& /*uid*/, PrivateXMLResult /*result*/ )
  {
  }

}
