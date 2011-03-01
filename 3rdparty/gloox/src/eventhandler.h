/*
  Copyright (c) 2008-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef EVENTHANDLER_H__
#define EVENTHANDLER_H__

namespace gloox
{

  class Event;

  /**
   * @brief An base class for event handlers.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class EventHandler
  {

    public:
      /**
       * Virtual Destructor.
       */
      virtual ~EventHandler() {}

      /**
       * This function gets called for Events this handler was registered for.
       * @param event The Event.
       */
      virtual void handleEvent( const Event& event ) = 0;

  };

}

#endif // EVENTHANDLER_H__
