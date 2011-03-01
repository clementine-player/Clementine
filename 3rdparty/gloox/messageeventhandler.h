/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MESSAGEEVENTHANDLER_H__
#define MESSAGEEVENTHANDLER_H__

#include "gloox.h"

namespace gloox
{

  class JID;

  /**
   * @brief A virtual interface that enables an object to be notified about
   * Message Events (XEP-0022).
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API MessageEventHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~MessageEventHandler() {}

      /**
       * Notifies the MessageEventHandler that an event has been raised by the remote
       * contact.
       * @param from The originator of the Event.
       * @param event The Event which has been raised.
       */
      virtual void handleMessageEvent( const JID& from, MessageEventType event ) = 0;

  };

}

#endif // MESSAGEEVENTHANDLER_H__
