/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef FLEXOFF_H__
#define FLEXOFF_H__

#include "clientbase.h"
#include "discohandler.h"
#include "flexoffhandler.h"
#include "iqhandler.h"
#include "stanzaextension.h"

namespace gloox
{

  /**
   * @brief An implementation of XEP-0013 (Flexible Offline Message Retrieval).
   *
   * Use the FlexibleOfflineHandler to receive results.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API FlexibleOffline : public DiscoHandler, public IqHandler
  {
    public:
      /**
       * Creates a new FlexibleOffline object that manages retrieval of offline messages.
       * @param parent The ClientBase to use for communication.
       */
      FlexibleOffline( ClientBase* parent );

      /**
       * Virtual Destructor.
       */
      virtual ~FlexibleOffline();

      /**
       * Initiates querying the server for Flexible Offline Message Retrieval-support.
       * The result is announced through the FlexibleOfflineHandler.
       * An application could cache the result on a per-server basis to eliminate the associated delay.
       */
      void checkSupport();

      /**
       * Asks the server for the number of stored offline messages.
       * The result is announced through the FlexibleOfflineHandler.
       */
      void getMsgCount();

      /**
       * Initiates fetching the offline message headers.
       * The result is announced through the FlexibleOfflineHandler.
       */
      void fetchHeaders();

      /**
       * Initiates fetching of one or more specific messages, or all messages.
       * The result is announced through the FlexibleOfflineHandler.
       * If the list of message nodes contains one or more nodes, the corresponding messages are
       * fetched. If the list is empty all messages are fetched (&lt;fetch&gt;).
       * @param msgs A list of message nodes to fetch.
       */
      void fetchMessages( const StringList& msgs )
        { messageOperation( FORequestMsgs, msgs ); }

      /**
       * Initiates removing of one or more specific messages, or all messages.
       * The result is announced through the FlexibleOfflineHandler.
       * If the list of message nodes contains one or more nodes, the corresponding messages are
       * removed. If the list is empty all messages are removed (&lt;purge&gt;).
       */
      void removeMessages( const StringList& msgs )
        { messageOperation( FORemoveMsgs, msgs ); }

      /**
       * Registers a FlexibleOfflineHandler as object that receives results of XEP-0013 queries.
       * Only one Handler at a time is possible.
       * @param foh The Handler object to register.
       */
      void registerFlexibleOfflineHandler( FlexibleOfflineHandler* foh );

      /**
       * Removes the registered handler.
       */
      void removeFlexibleOfflineHandler();

      // reimplemented from DiscoHandler
      virtual void handleDiscoInfo( const JID& from, const Disco::Info& info, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoItems( const JID& from, const Disco::Items& items, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoError( const JID& from, const Error* error, int context );

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:
#ifdef FLEXOFF_TEST
    public:
#endif
      class Offline : public StanzaExtension
      {
        public:
          /**
           * Constructs a new Offline object from the given Tag.
           * @param tag The Tag to parse.
           */
          Offline( const Tag* tag = 0 );

          /**
           * Constructs a new Offline object for the given context and messages.
           * @param context The context.
           * @param msgs The messages.
           */
          Offline( int context, const StringList& msgs );

          /**
           * Virtual destructor.
           */
          virtual ~Offline();

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Offline( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            return new Offline( *this );
          }

        private:
          int m_context;
          StringList m_msgs;
      };

      void messageOperation( int context, const StringList& msgs );

      enum FOContext
      {
        FOCheckSupport,
        FORequestNum,
        FORequestHeaders,
        FORequestMsgs,
        FORemoveMsgs
      };

      ClientBase* m_parent;
      FlexibleOfflineHandler* m_flexibleOfflineHandler;
  };

}

#endif // FLEXOFF_H__
