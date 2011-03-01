/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef LOGHANDLER_H__
#define LOGHANDLER_H__

#include "gloox.h"

#include <string>

namespace gloox
{

  /**
   * @brief A virtual interface which can be reimplemented to receive debug and log messages.
   *
   * @ref handleLog() is called for log messages.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.5
   */
  class GLOOX_API LogHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~LogHandler() {}

      /**
       * Reimplement this function if you want to receive the chunks of the conversation
       * between gloox and server or other debug info from gloox.
       * @param level The log message's severity.
       * @param area The log message's origin.
       * @param message The log message.
       */
      virtual void handleLog( LogLevel level, LogArea area, const std::string& message ) = 0;
  };

}

#endif // LOGHANDLER_H__
