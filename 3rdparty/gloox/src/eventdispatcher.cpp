/*
  Copyright (c) 2008-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "eventdispatcher.h"
#include "eventhandler.h"

namespace gloox
{

  EventDispatcher::EventDispatcher()
  {
  }

  EventDispatcher::~EventDispatcher()
  {
  }

  void EventDispatcher::dispatch( const Event& event, const std::string& context, bool remove )
  {
    typedef ContextHandlerMap::iterator Ei;
    std::pair<Ei, Ei> g = m_contextHandlers.equal_range( context );
    Ei it = g.first;
    Ei it2;
    while( it != g.second )
    {
      it2 = it++;
      (*it2).second->handleEvent( event );
      if( remove )
        m_contextHandlers.erase( it2 );
    }
  }

  void EventDispatcher::dispatch( const Event& event )
  {
    TypeHandlerMap::iterator it = m_typeHandlers.begin();
    for( ; it != m_typeHandlers.end(); ++it )
    {
      if( (*it).first == event.eventType() )
        (*it).second->handleEvent( event );
    }
  }

  void EventDispatcher::registerEventHandler( EventHandler* eh, const std::string& context )
  {
    if( !eh || context.empty() )
      return;

    m_contextHandlers.insert( std::make_pair( context, eh ) );
  }

  void EventDispatcher::removeEventHandler( EventHandler* eh )
  {
    ContextHandlerMap::iterator it = m_contextHandlers.begin();
    ContextHandlerMap::iterator it2;
    while( it != m_contextHandlers.end() )
    {
      it2 = it++;
      if( (*it2).second == eh )
        m_contextHandlers.erase( it2 );
    }
  }

}
