/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef PRIVACYMANAGER_H__
#define PRIVACYMANAGER_H__

#include "iqhandler.h"
#include "privacylisthandler.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class ClientBase;

  /**
   * @brief This class implements a manager for privacy lists as defined in section 10 of RFC 3921.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API PrivacyManager : public IqHandler
  {
    public:
      /**
       * Constructs a new PrivacyManager.
       * @param parent The ClientBase to use for communication.
       */
      PrivacyManager( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~PrivacyManager();

      /**
       * Stores the given list on the server. If a list with the given name exists, the existing
       * list is overwritten.
       * @param name The list's name.
       * @param list A non empty list of privacy items which describe the list.
       */
      std::string store( const std::string& name, const PrivacyListHandler::PrivacyList& list );

      /**
       * Triggers the request of the privacy lists currently stored on the server.
       */
      std::string requestListNames()
        { return operation( PLRequestNames, EmptyString ); }

      /**
       * Triggers the retrieval of the named privacy lists.
       * @param name The name of the list to retrieve.
       */
      std::string requestList( const std::string& name )
        { return operation( PLRequestList, name ); }

      /**
       * Removes a list by its name.
       * @param name The name of the list to remove.
       */
      std::string removeList( const std::string& name )
        { return operation( PLRemove, name ); }

      /**
       * Sets the named list as the default list, i.e. active by default after login.
       * @param name The name of the list to set as default.
       */
      std::string setDefault( const std::string& name )
        { return operation( PLDefault, name ); }

      /**
       * This function declines the use of any default list.
       */
      std::string unsetDefault()
        { return operation( PLUnsetDefault, EmptyString ); }

      /**
       * Sets the named list as active, i.e. active for this session
       * @param name The name of the list to set active.
       */
      std::string setActive( const std::string& name )
        { return operation( PLActivate, name ); }

      /**
       * This function declines the use of any active list.
       */
      std::string unsetActive()
        { return operation( PLUnsetActivate, EmptyString ); }

      /**
       * Use this function to register an object as PrivacyListHandler.
       * Only one PrivacyListHandler at a time is possible.
       * @param plh The object to register as handler for privacy list related events.
       */
      void registerPrivacyListHandler( PrivacyListHandler* plh )
        { m_privacyListHandler = plh; }

      /**
       * Use this function to clear the registered PrivacyListHandler.
       */
      void removePrivacyListHandler()
        { m_privacyListHandler = 0; }

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:
      enum IdType
      {
        PLRequestNames,
        PLRequestList,
        PLActivate,
        PLDefault,
        PLUnsetActivate,
        PLUnsetDefault,
        PLRemove,
        PLStore
      };

      class Query : public StanzaExtension
      {
        public:
          /**
           * Creates a new query for storing or requesting a
           * privacy list.
           * @param context The context of the list.
           * @param name The list's name.
           * @param list The list's (optional) content.
           */
          Query( IdType context, const std::string& name,
                 const PrivacyListHandler::PrivacyList& list = PrivacyListHandler::PrivacyList() );

          /**
           * Creates a new query from the given Tag.
           * @param tag The Tag to parse.
           */
          Query( const Tag* tag = 0 );

          /**
           * Virtual destructor.
           */
          virtual ~Query();

          /**
           * Returns the name of the active list, if given.
           * @return The active list's name.
           */
          const std::string& active() const { return m_active; }

          /**
           * Returns the name of the default list, if given.
           * @return The default list's name.
           */
          const std::string& def() const { return m_default; }

          /**
           * Returns a list of privacy items, if given.
           * @return A list of PrivacyItems.
           */
          const PrivacyListHandler::PrivacyList& items() const
            { return m_items; }

          /**
           * Returns a list of list names.
           * @return A list of list names.
           */
          const StringList& names() const { return m_names; }

          /**
           * A convenience function that returns the first name of the list that
           * names() would return, or an empty string.
           * @return A list name.
           */
          const std::string& name() const
          {
            if( m_names.empty())
              return EmptyString;
            else
              return (*m_names.begin());
          }

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
            return new Query( *this );
          }

        private:
          IdType m_context;
          StringList m_names;
          std::string m_default;
          std::string m_active;
          PrivacyListHandler::PrivacyList m_items;
      };

      std::string operation( IdType context, const std::string& name );

      ClientBase* m_parent;
      PrivacyListHandler* m_privacyListHandler;
  };

}

#endif // PRIVACYMANAGER_H__
