/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef IQHANDLER_H__
#define IQHANDLER_H__

#include "iq.h"

namespace gloox
{

  /**
   * @brief A virtual interface which can be reimplemented to receive IQ stanzas.
   *
   * Derived classes can be registered as IqHandlers with the Client.
   * Upon an incoming IQ packet @ref handleIq() will be called.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API IqHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~IqHandler() {}

      /**
       * Reimplement this function if you want to be notified about incoming IQs.
       * @param iq The complete IQ stanza.
       * @return Indicates whether a request of type 'get' or 'set' has been handled. This includes
       * the obligatory 'result' answer. If you return @b false, a 'error' will be sent.
       * @since 1.0
       */
      virtual bool handleIq( const IQ& iq ) = 0;

      /**
       * Reimplement this function if you want to be notified about
       * incoming IQs with a specific value of the @c id attribute. You
       * have to enable tracking of those IDs using Client::trackID().
       * This is usually useful for IDs that generate a positive reply, i.e.
       * &lt;iq type='result' id='reg'/&gt; where a namespace filter wouldn't
       * work.
       * @param iq The complete IQ stanza.
       * @param context A value to restore context, stored with ClientBase::trackID().
       * @note Only IQ stanzas of type 'result' or 'error' can arrive here.
       * @since 1.0
       */
      virtual void handleIqID( const IQ& iq, int context ) = 0;

  };

}

#endif // IQHANDLER_H__
