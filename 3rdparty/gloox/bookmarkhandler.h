/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef BOOKMARKHANDLER_H__
#define BOOKMARKHANDLER_H__

#include "macros.h"

#include <string>
#include <list>

namespace gloox
{

  /**
   * This describes a single bookmarked URL item.
   */
  struct BookmarkListItem
  {
    std::string name;             /**< A human readable name of the bookmark. */
    std::string url;              /**< The URL of the bookmark. */
  };

  /**
   * This describes a single bookmarked conference item.
   */
  struct ConferenceListItem
  {
    std::string name;             /**< A human readable name of the conference room. */
    std::string jid;              /**< The address of the room. */
    std::string nick;             /**< The nick name to use in this room. */
    std::string password;         /**< The password to use for a protected room. */
    bool autojoin;                /**< The conference shall be joined automatically on login. */
  };

  /**
   * A list of URL items.
   */
  typedef std::list<BookmarkListItem> BookmarkList;

  /**
   * A list of conference items.
   */
  typedef std::list<ConferenceListItem> ConferenceList;

  /**
   * @brief A virtual interface which can be reimplemented to receive bookmarks with help of a
   * BookmarkStorage object.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API BookmarkHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~BookmarkHandler() {}

      /**
       * This function is called when bookmarks arrive from the server.
       * @param bList A list of URL bookmarks.
       * @param cList A list of conference bookmarks.
       */
      virtual void handleBookmarks( const BookmarkList &bList, const ConferenceList &cList ) = 0;
  };

}

#endif // BOOKMARKHANDLER_H__
