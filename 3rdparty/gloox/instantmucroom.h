/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef INSTANTMUCROOM_H__
#define INSTANTMUCROOM_H__

#include "mucroom.h"

namespace gloox
{

  /**
   * @brief This class implements an instant MUC room.
   *
   * XEP version: 1.21
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API InstantMUCRoom : public MUCRoom
  {
    public:
      /**
       * Creates a new abstraction of a @b unique Multi-User Chat room. The room is not joined
       * automatically. Use join() to join the room, use leave() to leave it. See MUCRoom for
       * detailed info.
       * @param parent The ClientBase object to use for the communication.
       * @param nick The room's name and service plus the desired nickname in the form
       * room\@service/nick.
       * @param mrh The MUCRoomHandler that will listen to room events. May be 0 and may be specified
       * later using registerMUCRoomHandler(). However, without one, MUC is no joy.
       * @note To subsequently configure the room, use MUCRoom::registerMUCRoomConfigHandler().
       */
      InstantMUCRoom( ClientBase* parent, const JID& nick, MUCRoomHandler* mrh );

      /**
       * Virtual Destructor.
       */
      virtual ~InstantMUCRoom();

    protected:
      // reimplemented from MUCRoom (acknowledges instant room creation w/o a
      // call to the MUCRoomConfigHandler)
      virtual bool instantRoomHook() const { return true; }

  };

}

#endif // INSTANTMUCROOM_H__
