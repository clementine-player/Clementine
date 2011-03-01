/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "bookmarkstorage.h"
#include "clientbase.h"


namespace gloox
{

  BookmarkStorage::BookmarkStorage( ClientBase* parent )
    : PrivateXML( parent ),
      m_bookmarkHandler( 0 )
  {
  }

  BookmarkStorage::~BookmarkStorage()
  {
  }

  void BookmarkStorage::storeBookmarks( const BookmarkList& bList, const ConferenceList& cList )
  {
    Tag* s = new Tag( "storage" );
    s->addAttribute( XMLNS, XMLNS_BOOKMARKS );

    BookmarkList::const_iterator itb = bList.begin();
    for( ; itb != bList.end(); ++itb )
    {
      Tag* i = new Tag( s, "url", "name", (*itb).name );
      i->addAttribute( "url", (*itb).url );
    }

    ConferenceList::const_iterator itc = cList.begin();
    for( ; itc != cList.end(); ++itc )
    {
      Tag* i = new Tag( s, "conference", "name", (*itc).name );
      i->addAttribute( "jid", (*itc).jid );
      i->addAttribute( "autojoin", (*itc).autojoin ? "true" : "false" );

      new Tag( i, "nick", (*itc).nick );
      new Tag( i, "password", (*itc).password );
    }

    storeXML( s, this );
  }

  void BookmarkStorage::requestBookmarks()
  {
    requestXML( "storage", XMLNS_BOOKMARKS, this );
  }

  void BookmarkStorage::handlePrivateXML( const Tag* xml )
  {
    if( !xml )
      return;

    BookmarkList bList;
    ConferenceList cList;
    const TagList& l = xml->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "url" )
      {
        const std::string& url = (*it)->findAttribute( "url" );
        const std::string& name = (*it)->findAttribute( "name" );

        if( !url.empty() && !name.empty() )
        {
          BookmarkListItem item;
          item.url = url;
          item.name = name;
          bList.push_back( item );
        }
      }
      else if( (*it)->name() == "conference" )
      {
        const std::string& jid = (*it)->findAttribute( "jid" );
        const std::string& name = (*it)->findAttribute( "name" );

        if( !jid.empty() && !name.empty() )
        {
          const std::string& join = (*it)->findAttribute( "autojoin" );
          ConferenceListItem item;
          item.jid = jid;
          item.name = name;
          const Tag* nick = (*it)->findChild( "nick" );
          if( nick )
            item.nick = nick->cdata();
          const Tag* pwd = (*it)->findChild( "password" );
          if( pwd )
            item.password = pwd->cdata();
          item.autojoin = ( join == "true" || join == "1" );
          cList.push_back( item );
        }
      }
    }

    if( m_bookmarkHandler )
      m_bookmarkHandler->handleBookmarks( bList, cList );
  }

  void BookmarkStorage::handlePrivateXMLResult( const std::string& /*uid*/, PrivateXMLResult /*result*/ )
  {
  }

}
