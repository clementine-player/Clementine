/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SEARCHHANDLER_H__
#define SEARCHHANDLER_H__

#include "stanza.h"

#include <string>

namespace gloox
{

  class DataForm;

  /**
   * Holds all the possible fields a server may require for searching according
   * to Section 7, XEP-0055.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class SearchFieldStruct
  {
    public:
      /**
       *
       */
      SearchFieldStruct() {}

      /**
       *
       */
      SearchFieldStruct( const std::string& first, const std::string& last, const std::string& nick,
                         const std::string& email )
        : m_first( first ), m_last( last ), m_nick( nick ), m_email( email )
      {}

      /**
       *
       */
      SearchFieldStruct( const Tag* tag )
      {
        if( !tag || tag->name() != "item" || !tag->hasAttribute( "jid" ) )
          return;

        m_jid.setJID( tag->findAttribute( "jid" ) );
        const TagList& l = tag->children();
        TagList::const_iterator it = l.begin();
        for( ; it != l.end(); ++it )
        {
          if( (*it)->name() == "first" )
            m_first = (*it)->cdata();
          else if( (*it)->name() == "last" )
            m_last = (*it)->cdata();
          else if( (*it)->name() == "email" )
            m_email = (*it)->cdata();
          else if( (*it)->name() == "nick" )
            m_nick = (*it)->cdata();
        }
      }

      /**
       *
       */
      ~SearchFieldStruct() {}

      /**
       *
       */
      const std::string first() const { return m_first; }

      /**
       *
       */
      const std::string last() const { return m_last; }

      /**
       *
       */
      const std::string email() const { return m_email; }

      /**
       *
       */
      const std::string nick() const { return m_nick; }

      /**
       *
       */
      Tag* tag() const
      {
        Tag* t = new Tag( "item" );
        t->addAttribute( "jid", m_jid.bare() );
        new Tag( t, "first", m_first );
        new Tag( t, "last", m_last );
        new Tag( t, "nick", m_nick );
        new Tag( t, "email", m_email );
        return t;
      }

    private:
      std::string m_first;              /**< User's first name. */
      std::string m_last;               /**< User's last name. */
      std::string m_nick;               /**< User's nickname. */
      std::string m_email;              /**< User's email. */
      JID m_jid;                        /**< User's JID. */
  };

  /**
   * The possible fields of a XEP-0055 user search.
   */
  enum SearchFieldEnum
  {
    SearchFieldFirst    = 1,        /**< Search in first names. */
    SearchFieldLast     = 2,        /**< Search in last names. */
    SearchFieldNick     = 4,        /**< Search in nicknames. */
    SearchFieldEmail    = 8         /**< Search in email addresses. */
  };

  /**
   * A list of directory entries returned by a search.
   */
  typedef std::list<const SearchFieldStruct*> SearchResultList;

  /**
   * @brief A virtual interface that enables objects to receive Jabber Search (XEP-0055) results.
   *
   * A class implementing this interface can receive the result of a Jabber Search.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8.5
   */
  class GLOOX_API SearchHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~SearchHandler() {}

      /**
       * This function is called to announce the searchable fields a directory supports. It is the result
       * of a call to @link gloox::Search::fetchSearchFields Search::fetchSearchFields() @endlink.
       * @param directory The directory that was queried.
       * @param fields Bit-wise ORed SearchFieldEnum values.
       * @param instructions Plain-text instructions for the end user.
       */
      virtual void handleSearchFields( const JID& directory, int fields,
                                       const std::string& instructions ) = 0;

      /**
       * This function is called to announce the searchable fields a directory supports. It is the result
       * of a call to @link gloox::Search::fetchSearchFields Search::fetchSearchFields() @endlink.
       * @param directory The directory that was queried.
       * @param form A DataForm describing the valid searchable fields. Do not delete the form.
       */
      virtual void handleSearchFields( const JID& directory, const DataForm* form ) = 0;

     /**
      * This function is called to let the SearchHandler know about the results of the search.
      * @param directory The searched directory.
      * @param resultList A list of SearchFieldStructs. May be empty.
      */
      virtual void handleSearchResult( const JID& directory, const SearchResultList& resultList ) = 0;

      /**
       * This function is called to let the SearchHandler know about the result of the search.
       * @param directory The searched directory.
       * @param form A DataForm containing the search results. Do not delete the form.
       */
      virtual void handleSearchResult( const JID& directory, const DataForm* form ) = 0;

      /**
       * This function is called if a error occured as a result to a search or search field request.
       * @param directory The queried/searched directory.
       * @param error The error. May be 0.
       */
      virtual void handleSearchError( const JID& directory, const Error* error ) = 0;

  };

}

#endif // SEARCHHANDLER_H__
