/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef CONNECTIONDATAHANDLER_H__
#define CONNECTIONDATAHANDLER_H__

#include "gloox.h"

#include <string>

namespace gloox
{

  class ConnectionBase;

  /**
   * @brief This is an abstract base class to receive events from a ConnectionBase-derived object.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API ConnectionDataHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~ConnectionDataHandler() {}

      /**
       * This function is called for received from the underlying transport.
       * @param connection The connection that received the data.
       * @param data The data received.
       */
      virtual void handleReceivedData( const ConnectionBase* connection, const std::string& data ) = 0;

      /**
       * This function is called when e.g. the raw TCP connection was established.
       * @param connection The connection.
       */
      virtual void handleConnect( const ConnectionBase* connection ) = 0;

      /**
       * This connection is called when e.g. the raw TCP connection was closed.
       * @param connection The connection.
       * @param reason The reason for the disconnect.
       */
      virtual void handleDisconnect( const ConnectionBase* connection, ConnectionError reason ) = 0;
  };

}

#endif // CONNECTIONDATAHANDLER_H__
