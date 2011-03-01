/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ADHOCHANDLER_H__
#define ADHOCHANDLER_H__

#include "adhoc.h"

namespace gloox
{

  /**
   * @brief A virtual interface for an Ad-hoc Command users according to XEP-0050.
   *
   * Derived classes can be registered with the Adhoc object to receive notifications
   * about Adhoc Commands remote entities support.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API AdhocHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~AdhocHandler() {}

      /**
       * This function is called in response to a call to Adhoc::checkSupport().
       * @param remote The queried remote entity's JID.
       * @param support Whether the remote entity supports Adhoc Commands.
       */
      virtual void handleAdhocSupport( const JID& remote, bool support ) = 0;

      /**
       * This function is called in response to a call to Adhoc::getCommands()
       * and delivers a list of supported commands.
       * @param remote The queried remote entity's JID.
       * @param commands A map of supported commands and their human-readable name.
       * The map may be empty.
       */
      virtual void handleAdhocCommands( const JID& remote, const StringMap& commands ) = 0;

      /**
       * This function is called in response to a call to Adhoc::getCommands() or
       * Adhoc::checkSupport() or Adhoc::execute() in case the respective request returned
       * an error.
       * @param remote The queried remote entity's JID.
       * @param error The error condition. May be 0.
       */
      virtual void handleAdhocError( const JID& remote, const Error* error ) = 0;

      /**
       * This function is called in response to a remote command execution.
       * @param remote The remote entity's JID.
       * @param command The command being executed.
       */
      virtual void handleAdhocExecutionResult( const JID& remote, const Adhoc::Command& command ) = 0;
  };

}

#endif // ADHOCHANDLER_H__
