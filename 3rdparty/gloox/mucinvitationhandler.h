/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef MUCINVITATIONHANDLER_H__
#define MUCINVITATIONHANDLER_H__

#include "clientbase.h"
#include "macros.h"
#include "jid.h"
#include "mucroom.h"

#include <string>

namespace gloox
{

  /**
   * @brief A handler that can be used to receive invitations to MUC rooms.
   *
   * Register a derived class with ClientBase::registerMUCInvitationHandler().
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API MUCInvitationHandler
  {
    public:
      /**
       * Constructor. Prepares the given ClientBase for receiving MUC invitations..
       * @param parent A ClientBase instance to prepare.
       */
      MUCInvitationHandler( ClientBase* parent )
      {
        if( parent )
          parent->registerStanzaExtension( new MUCRoom::MUCUser() );
      }

      /**
       * Virtual Destructor.
       */
      virtual ~MUCInvitationHandler() {}

      /**
       * This function is called for incoming invitations to MUC rooms.
       * @param room The JID of the room you're being invited to.
       * @param from The JID of the inviter.
       * @param reason A reason for the invitation.
       * @param body The body of the message. May contain a MUC-service generated invitation message.
       * @param password Optionally, a password for the room.
       * @param cont Indicates whether or not the multi-user chat is a continuation of a private chat.
       * @param thread An optional thread identifier in case this is a
       * continued chat.
       */
      virtual void handleMUCInvitation( const JID& room, const JID& from, const std::string& reason,
                                        const std::string& body, const std::string& password,
                                        bool cont, const std::string& thread ) = 0;
  };

}

#endif // MUCINVITATIONHANDLER_H__
