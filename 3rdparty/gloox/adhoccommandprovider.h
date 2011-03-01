/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ADHOCCOMMANDPROVIDER_H__
#define ADHOCCOMMANDPROVIDER_H__

#include "tag.h"
#include "jid.h"
#include "adhoc.h"

#include <list>
#include <map>
#include <string>

namespace gloox
{

  /**
   * @brief A virtual interface for an Ad-hoc Command Provider according to XEP-0050.
   *
   * Derived classes can be registered as Command Providers with the Adhoc object.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API AdhocCommandProvider
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~AdhocCommandProvider() {}

      /**
       * This function is called when an Ad-hoc Command needs to be handled.
       * The callee is responsible for the whole command execution, i.e. session
       * handling etc.
       * @param from The sender of the command request.
       * @param command The name of the command to be executed.
       * @param sessionID The session ID. Either newly generated or taken from the command.
       * When responding, its value must be passed to Adhoc::Command's constructor.
       */
      virtual void handleAdhocCommand( const JID& from, const Adhoc::Command& command,
                                       const std::string& sessionID ) = 0;

      /**
       * This function gets called for each registered command when a remote
       * entity requests the list of available commands.
       * @param from The requesting entity.
       * @param command The command's name.
       * @return @b True if the remote entity is allowed to see the command, @b false if not.
       * @note The return value of this function does not influence
       * the execution of a command. That is, you have to
       * implement additional access control at the execution
       * stage.
       * @note This function should not block.
       */
      virtual bool handleAdhocAccessRequest( const JID& from, const std::string& command )
      {
        (void)from;
        (void)command;
        return true;
      }

  };

}

#endif // ADHOCCOMMANDPROVIDER_H__
