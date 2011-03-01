/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef BOOKMARKSTORAGE_H__
#define BOOKMARKSTORAGE_H__

#include "macros.h"

#include "bookmarkhandler.h"
#include "privatexml.h"
#include "privatexmlhandler.h"

#include <string>
#include <list>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an implementation of XEP-0048 (Bookmark Storage).
   *
   * You can use this class to store bookmarks to multi-user chat rooms or ordinary URLs
   * on the server (and to retrieve them later on).
   * To retrieve all stored bookmarks for the current user you have to create a class which
   * inherits from BookmarkHandler. This handler receives retrieved bookmarks.
   *
   * @code
   * class MyClass : public BookmarkHandler
   * {
   *   public:
   *     // ...
   *     void myFuncRetrieve();
   *     void myFuncStore();
   *     void handleBookmarks( const BookmarkList &bList, const ConferenceList &cList );
   *
   *   private:
   *     BookmarkStorage* m_bs;
   *     BookmarkList m_bList;
   *     ConferenceList m_cList;
   * };
   *
   * void MyClass::myFuncRetrieve()
   * {
   *   m_bs = new BookmarkStorage( m_client );
   *   m_bs->requestBookmarks();
   * }
   *
   * void MyClass::handleBookmarks( const BookmarkList &bList, const ConferenceList &cList )
   * {
   *   m_bList = bList;
   *   m_cList = cList;
   * }
   * @endcode
   *
   *
   * To store additional bookmarks you have to fetch the currently stored ones first,
   * add your new bookmark to the list, and transfer them all together back to the
   * server. This protocol does not support storage of 'deltas', that is, when saving
   * bookmarks all previously saved bookmarks are overwritten.
   *
   * @code
   * void MyClass::myFuncStore()
   * {
   *   BookmarkListItem bi;
   *   bi.url = "http://www.jabber.org";
   *   bi.name = "my favourite IM protocol";
   *   m_bList.push_back( bi );
   *
   *   conferenceListItem ci
   *   ci.name = "jabber/xmpp development room";
   *   ci.jid = "jdev@conference.jabber.org";
   *   ci.nick = "myNick";
   *   ci.password = EmptyString;
   *   ci.autojoin = true;
   *   m_cList.push_back( ci );
   *
   *   m_bs->storeBookmarks( m_bList, m_cList );
   * }
   * @endcode
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API BookmarkStorage : public PrivateXML, public PrivateXMLHandler
  {
    public:
      /**
       * Constructs a new BookmarkStorage object.
       * @param parent The ClientBase to use for communication.
       */
      BookmarkStorage( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~BookmarkStorage();

      /**
       * Use this function to store a number of URL and conference bookmarks on the server.
       * Make sure you store the whole set of bookmarks, not a 'delta'.
       * @param bList A list of URLs to store.
       * @param cList A list of conferences to store.
       */
      void storeBookmarks( const BookmarkList& bList, const ConferenceList& cList );

      /**
       * Use this function to initiate retrieval of bookmarks. Use registerBookmarkHandler()
       * to register an object which will receive the lists of bookmarks.
       */
      void requestBookmarks();

      /**
       * Use this function to register a BookmarkHandler.
       * @param bmh The BookmarkHandler which shall receive retrieved bookmarks.
       */
      void registerBookmarkHandler( BookmarkHandler* bmh )
        { m_bookmarkHandler = bmh; }

      /**
       * Use this function to un-register the BookmarkHandler.
       */
      void removeBookmarkHandler()
        { m_bookmarkHandler = 0; }

      // reimplemented from PrivateXMLHandler
      virtual void handlePrivateXML( const Tag* xml );

      // reimplemented from PrivateXMLHandler
      virtual void handlePrivateXMLResult( const std::string& uid, PrivateXMLResult pxResult );

    private:
      BookmarkHandler* m_bookmarkHandler;
  };

}

#endif // BOOKMARKSTORAGE_H__
