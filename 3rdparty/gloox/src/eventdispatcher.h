/*
  Copyright (c) 2008-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef EVENTDISPATCHER_H__
#define EVENTDISPATCHER_H__

#include "event.h"

#include <map>
#include <string>

namespace gloox
{

  class EventHandler;

  /**
   * @brief An Event dispatcher.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class EventDispatcher
  {

    public:
      /**
       * Creates a new EventDispatcher object. You should not need to use this class directly.
       */
      EventDispatcher();

      /**
       * Virtual Destructor.
       */
      virtual ~EventDispatcher();

      /**
       * Looks for handlers for the given Event, and removes the handlers if requested.
       * @param event The Event to dispatch.
       * @param context An identifier that limits the EventHandlers that will get notified to
       * those that are specifically interested in this context.
       * @param remove Whether or not to remove the context from the list of known contexts. Useful for
       * IQ IDs.
       */
      void dispatch( const Event& event, const std::string& context, bool remove );

      /**
       * Looks for handlers for the given Event, identified by its type.
       * @param event The event to dispatch.
       */
      void dispatch( const Event& event );

      /**
       * Registers the given EventHandler to be notified about Events with the given context.
       * The context will usually be an IQ ID.
       * @param eh The EventHandler to register.
       * @param context The context to register the EventHandler for.
       */
      void registerEventHandler( EventHandler* eh, const std::string& context );

      /**
       * Removes the given EventHandler.
       * @param eh The EventHandler to remove.
       */
      void removeEventHandler( EventHandler* eh );

    private:
      typedef std::multimap<const std::string, EventHandler*> ContextHandlerMap;
      typedef std::multimap<Event::EventType, EventHandler*> TypeHandlerMap;

      ContextHandlerMap m_contextHandlers;
      TypeHandlerMap m_typeHandlers;

  };

}

#endif // EVENTDISPATCHER_H__
