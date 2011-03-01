/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef LASTACTIVITYHANDLER_H__
#define LASTACTIVITYHANDLER_H__

#include "gloox.h"
#include "jid.h"

namespace gloox
{

  /**
   * @brief This is an virtual interface that, once reimplemented, allows to receive the
   * results of Last-Activity-queries to other entities.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.6
   */
  class GLOOX_API LastActivityHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~LastActivityHandler() {}

      /**
       * This function is called when a positive result of a query arrives.
       * @param jid The JID of the queried contact.
       * @param seconds The idle time or time of last presence of the contact. (Depends
       * on the JID, check the spec.)
       * @param status If the contact is offline, this is the last presence status message. May be empty.
       */
      virtual void handleLastActivityResult( const JID& jid, long seconds, const std::string& status ) = 0;

      /**
       * This function is called when an error is returned by the queried antity.
       * @param jid The queried entity's address.
       * @param error The reported error.
       */
      virtual void handleLastActivityError( const JID& jid, StanzaError error ) = 0;

  };

}

#endif // LASTACTIVITYHANDLER_H__
