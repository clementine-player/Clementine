/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CHATSTATEHANDLER_H__
#define CHATSTATEHANDLER_H__

#include "gloox.h"

namespace gloox
{

  class JID;

  /**
   * @brief A virtual interface that enables an object to be notified about
   * a remote entity's Chat States (XEP-0085).
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API ChatStateHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~ChatStateHandler() {}

      /**
       * Notifies the ChatStateHandler that a different chat state has been set by the remote
       * contact.
       * @param from The originator of the Event.
       * @param state The chat state set by the remote entity.
       */
      virtual void handleChatState( const JID& from, ChatStateType state ) = 0;

  };

}

#endif // CHATSTATEHANDLER_H__
