/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MESSAGEHANDLER_H__
#define MESSAGEHANDLER_H__

#include "macros.h"

namespace gloox
{

  class MessageSession;
  class Message;

  /**
   * @brief A virtual interface which can be reimplemented to receive incoming message stanzas.
   *
   * Derived classes can be registered as MessageHandlers with a ClientBase or MessageSession instance.
   * Upon an incoming Message packet @ref handleMessage() will be called. If registered with a
   * ClientBase this happens for every incoming message, regardless of the sender. With a MessageSession
   * the registered handler will receive all messages originating from the Session's contact. See
   * MessageSession for more details.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API MessageHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~MessageHandler() {}

      /**
       * Reimplement this function if you want to be notified about
       * incoming messages.
       * @param msg The complete Message.
       * @param session If this MessageHandler is used with a MessageSession, this parameter
       * holds a pointer to that MessageSession.
       * @since 1.0
       */
      virtual void handleMessage( const Message& msg, MessageSession* session = 0 ) = 0;

  };

}

#endif // MESSAGEHANDLER_H__
