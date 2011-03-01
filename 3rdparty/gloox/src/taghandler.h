/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef TAGHANDLER_H__
#define TAGHANDLER_H__

#include "tag.h"

namespace gloox
{

  /**
   * @brief A virtual interface which can be reimplemented to receive non-XMPP Core stanzas.
   *
   * Derived classes can be registered as TagHandlers with the ClientBase.
   * A TagHandler can handle arbitrary elements not defined by RFC 3920, XMPP Core.
   *
   * It can also be used to handle Tags emitted by Parser.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API TagHandler
  {
     public:
      /**
       * Virtual Destructor.
       */
       virtual ~TagHandler() {}

       /**
        * This function is called when a registered XML element arrives.
        * As with every handler in gloox, the Tag is going to be deleted after this function returned.
        * If you need a copy afterwards, create it using Tag::clone().
        * @param tag The complete Tag.
        */
       virtual void handleTag( Tag* tag ) = 0;
  };

}

#endif // TAGHANDLER_H__
