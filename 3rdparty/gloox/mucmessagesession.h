/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MUCMESSAGESESSION_H__
#define MUCMESSAGESESSION_H__

#include "messagesession.h"

namespace gloox
{

  class ClientBase;

  /**
   * @brief This is a MessageSession, adapted to be used in a MUC context.
   *
   * This class is used internally by MUCRoom. You should not need to use it directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API MUCMessageSession : public MessageSession
  {
    public:
      /**
       * Creates a new MUCMessageSession.
       * @param parent The ClientBase to use for communication.
       * @param jid The @b bare JID of the MUC room.
       */
      MUCMessageSession( ClientBase* parent, const JID& jid );

      /**
       * Virtual Destructor.
       */
      virtual ~MUCMessageSession();

      /**
       * Use this function to send a message to all room occupants.
       * @param message The message to send.
       */
      virtual void send( const std::string& message );

      /**
       * Use this function to set a new room subject.
       * @param subject The new room subject.
       */
      virtual void setSubject( const std::string& subject );

      // reimplemented from MessageSession
      virtual void handleMessage( Message& msg );

  };

}

#endif // MUCMESSAGESESSION_H__
