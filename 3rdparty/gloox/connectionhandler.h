/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef CONNECTIONHANDLER_H__
#define CONNECTIONHANDLER_H__

#include "connectionbase.h"

namespace gloox
{

  /**
   * @brief This is an abstract base class to receive incoming connection attempts. Do not
   * confuse this with ConnectionListener, which is used with XMPP streams and has a
   * completely different meaning.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API ConnectionHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~ConnectionHandler() {}

      /**
       * This function is called to receive an incoming connection.
       * @param server The server that the connection was made to.
       * @param connection The incoming connection.
       */
      virtual void handleIncomingConnection( ConnectionBase* server, ConnectionBase* connection ) = 0;

  };

}

#endif // CONNECTIONHANDLER_H__
