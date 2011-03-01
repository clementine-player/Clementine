/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef SEARCH_H__
#define SEARCH_H__

#include "gloox.h"
#include "searchhandler.h"
#include "discohandler.h"
#include "iqhandler.h"
#include "stanzaextension.h"
#include "dataform.h"

#include <string>

namespace gloox
{

  class ClientBase;
  class IQ;
  class Disco;

  /**
   * @brief An implementation of XEP-0055 (Jabber Search)
   *
   * To perform a search in a directory (e.g., a User Directory):
   *
   * @li Inherit from SearchHandler and implement the virtual functions.
   * @li Create a new Search object.
   * @li Ask the directory for the supported fields using fetchSearchFields(). Depending on the directory,
   * the result can be either an integer (bit-wise ORed supported fields) or a DataForm.
   * @li Search by either using a DataForm or the SearchFieldStruct.
   * @li The results can be either a (empty) list of SearchFieldStructs or a DataForm.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8.5
   */
  class GLOOX_API Search : public IqHandler
  {

    public:
      /**
       * Creates a new Search object.
       * @param parent The ClientBase to use.
       */
      Search( ClientBase* parent );

      /**
       * Virtual Destructor.
       */
      ~Search();

      /**
       * Use this function to check which fields the directory supports.
       * @param directory The (user) directory to fetch the available/searchable fields from.
       * @param sh The SearchHandler to notify about the results.
       */
      void fetchSearchFields( const JID& directory, SearchHandler* sh );

      /**
       * Initiates a search on the given directory, with the given data form. The given SearchHandler
       * is notified about the results.
       * @param directory The (user) directory to search.
       * @param form The DataForm contains the phrases the user wishes to search for.
       * Search will delete the form eventually.
       * @param sh The SearchHandler to notify about the results.
       */
      void search( const JID& directory, DataForm* form, SearchHandler* sh );

      /**
       * Initiates a search on the given directory, with the given phrases. The given SearchHandler
       * is notified about the results.
       * @param directory The (user) directory to search.
       * @param fields Bit-wise ORed FieldEnum values describing the valid (i.e., set) fields in
       * the @b values parameter.
       * @param values Contains the phrases to search for.
       * @param sh The SearchHandler to notify about the results.
       */
      void search( const JID& directory, int fields, const SearchFieldStruct& values, SearchHandler* sh );

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    protected:
      enum IdType
      {
        FetchSearchFields,
        DoSearch
      };

      typedef std::map<std::string, SearchHandler*> TrackMap;
      TrackMap m_track;

      ClientBase* m_parent;
      Disco* m_disco;

    private:
#ifdef SEARCH_TEST
    public:
#endif
      /**
       * @brief A wrapping class for the XEP-0055 &lt;query&gt; element.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class Query : public StanzaExtension
      {
        public:
          /**
           * Creates a new object that can be used to carry out a search.
           * @param form A DataForm containing the search terms.
           */
          Query( DataForm* form );

          /**
           * Creates a new object that can be used to carry out a search.
           * @param fields Bit-wise ORed FieldEnum values describing the valid (i.e., set)
           * fields in the @b values parameter.
           * @param values Contains the phrases to search for.
           */
          Query( int fields, const SearchFieldStruct& values );

          /**
           * Creates a new object that can be used to request search fields.
           * Optionally, it can parse the given Tag.
           * @param tag The Tag to parse.
           */
          Query( const Tag* tag = 0 );

          /**
           * Virtual Destructor.
           */
          virtual ~Query();

          /**
           * Returns the contained search form, if any.
           * @return The search form. May be 0.
           */
          const DataForm* form() const { return m_form; }

          /**
           * Returns the search instructions, if given
           * @return The search instructions.
           */
          const std::string& instructions() const { return m_instructions; }

          /**
           * Returns the search fields, if set.
           * @return The search fields.
           */
          int fields() const { return m_fields; }

          /**
           * Returns the search's result, if available in legacy form. Use form() otherwise.
           * @return The search's result.
           */
          const SearchResultList& result() const { return m_srl; }

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Query( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            Query* q = new Query();
            q->m_form = m_form ? new DataForm( *m_form ) : 0;
            q->m_fields = m_fields;
            q->m_values = m_values;
            q->m_instructions = m_instructions;
            SearchResultList::const_iterator it = m_srl.begin();
            for( ; it != m_srl.end(); ++it )
              q->m_srl.push_back( new SearchFieldStruct( *(*it) ) );
            return q;
          }

        private:
#ifdef SEARCH_TEST
        public:
#endif
          DataForm* m_form;
          int m_fields;
          SearchFieldStruct m_values;
          std::string m_instructions;
          SearchResultList m_srl;
      };

  };

}

#endif // SEARCH_H__
