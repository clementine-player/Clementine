/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MESSAGEVENTFILTER_H__
#define MESSAGEVENTFILTER_H__

#include "messagefilter.h"
#include "gloox.h"

namespace gloox
{

  class Tag;
  class Message;
  class MessageEventHandler;
  class MessageSession;

  /**
   * @brief This class adds Message Event (XEP-0022) support to a MessageSession.
   *
   * This implementation of Message Events is fully transparent to the user of the class.
   * If the remote entity does not request message events, MessageEventFilter will not send
   * any, even if the user requests it. (This is required by the protocol specification.)
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API MessageEventFilter : public MessageFilter
  {
    public:
      /**
       * Contstructs a new Message Event filter for a MessageSession.
       * @param parent The MessageSession to decorate.
       */
      MessageEventFilter( MessageSession* parent );

      /**
       * Virtual destructor.
       */
      virtual ~MessageEventFilter();

      /**
       * Use this function to raise an event as defined in XEP-0022.
       * @note The Spec states that Message Events shall not be sent to an entity
       * which did not request them. Reasonable effort is taken in this function to
       * avoid spurious event sending. You should be safe to call this even if Message
       * Events were not requested by the remote entity. However,
       * calling raiseMessageEvent( MESSAGE_EVENT_COMPOSING ) for every keystroke still is
       * discouraged. ;)
       * @param event The event to raise.
       */
      void raiseMessageEvent( MessageEventType event );

      /**
       * The MessageEventHandler registered here will receive Message Events according
       * to XEP-0022.
       * @param meh The MessageEventHandler to register.
       */
      void registerMessageEventHandler( MessageEventHandler* meh );

      /**
       * This function clears the internal pointer to the MessageEventHandler.
       * Message Events will not be delivered anymore after calling this function until another
       * MessageEventHandler is registered.
       */
      void removeMessageEventHandler();

      // reimplemented from MessageFilter
      virtual void decorate( Message& msg );

      // reimplemented from MessageFilter
      virtual void filter( Message& msg );

    private:
      MessageEventHandler* m_messageEventHandler;
      std::string m_lastID;
      int m_requestedEvents;
      MessageEventType m_lastSent;
      bool m_disable;

  };

}

#endif // MESSAGEVENTFILTER_H__
