/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MESSAGEFILTER_H__
#define MESSAGEFILTER_H__

#include "messagesession.h"

namespace gloox
{

  class Message;

  /**
   * @brief Virtual base class for message filters.
   *
   * A message filter is fed with all messages passing through a MessageSession. It can
   * modify the XML/XMPP structure and/or the message content at will. Messages arriving
   * from the server as well as messages sent to the server can be altered.
   *
   * Messages to be sent out are presented to the filter via the decorate() function, incoming
   * messages can be filtered in the -- filter() method.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API MessageFilter
  {

    public:
      /**
       * Constructor.
       * @param parent The MessageSession to attach to.
       */
      MessageFilter( MessageSession* parent );

      /**
       * Virtual Destructor.
       */
      virtual ~MessageFilter();

      /**
       * Attaches this MessageFilter to the given MessageSession and hooks it into
       * the session's filter chain.
       * If this filter was attached to a different MessageSession before, it is
       * unregistered there prior to registering it with the new session.
       * @param session The MessageSession to hook into.
       */
      virtual void attachTo( MessageSession* session );

      /**
       * This function receives a message right before it is sent out (there may be other filters
       * which get to see the message after this filter, though).
       * @param msg The tag to decorate. It contains the message to be sent.
       */
      virtual void decorate( Message& msg ) = 0;

      /**
       * This function receives a message stanza right after it was received (there may be other filters
       * which got to see the stanza before this filter, though).
       * @param msg The complete message stanza.
       */
      virtual void filter( Message& msg ) = 0;

    protected:
      void send( Message& msg ) { if( m_parent ) m_parent->send( msg ); }

      MessageSession* m_parent;

  };

}

#endif // MESSAGEFILTER_H__
