/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIHANDLER_H__
#define SIHANDLER_H__

#include "macros.h"
#include "simanager.h"

#include <string>

namespace gloox
{

  class IQ;
  class Tag;
  class JID;

  /**
   * @brief An abstract base class to handle results of outgoing SI requests, i.e. you requested a stream
   * (using SIManager::requestSI()) to send a file to a remote entity.
   *
   * You should usually not need to use this class directly, unless your profile is not supported
   * by gloox.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIHandler
  {

    public:
      /**
       * Virtual destructor.
       */
      virtual ~SIHandler() {}

      /**
       * This function is called to handle results of outgoing SI requests, i.e. you requested a stream
       * (using SIManager::requestSI()) to send a file to a remote entity.
       * @param from The remote SI receiver.
       * @param to The SI requestor. Usually oneself. Used in component scenario.
       * @param sid The stream ID.
       * @param si The request's complete SI.
       */
      virtual void handleSIRequestResult( const JID& from, const JID& to, const std::string& sid,
                                          const SIManager::SI& si ) = 0;

      /**
       * This function is called to handle a request error or decline.
       * @param iq The complete error stanza.
       * @param sid The request's SID.
       */
      virtual void handleSIRequestError( const IQ& iq, const std::string& sid ) = 0;

  };

}

#endif // SIHANDLER_H__
