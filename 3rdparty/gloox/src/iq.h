/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef IQ_H__
#define IQ_H__

#include "stanza.h"
#include "gloox.h"

#include <string>

namespace gloox
{

  class JID;

  /**
   * @brief An abstraction of an IQ stanza.
   *
   * @author Vincent Thomasset
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API IQ : public Stanza
  {

    friend class ClientBase;

    public:

      /**
       * Describes the different valid IQ types.
       */
      enum IqType
      {
        Get = 0,                    /**< The stanza is a request for information or requirements. */
        Set,                        /**< The stanza provides required data, sets new values, or
                                     * replaces existing values. */
        Result,                     /**< The stanza is a response to a successful get or set request. */
        Error,                      /**< An error has occurred regarding processing or delivery of a
                                     * previously-sent get or set (see Stanza Errors (Section 9.3)). */
        Invalid                     /**< The stanza is invalid */
      };

      /**
       * Creates an IQ Query.
       * @param type The desired IqType.
       * @param to The intended receiver.
       * @param id The request's ID. Usually obtained from ClientBase::getID(). Optional,
       * will be added by ClientBase if the IQ is sent by means of
       * @link gloox::ClientBase::send( IQ&, IqHandler*, int, bool ) send( IQ&, IqHandler*, int, bool ) @endlink.
       * You should only need to pass this when creating a reply (i.e. an IQ of type Result or Error).
       */
      IQ( IqType type, const JID& to, const std::string& id = EmptyString );

      /**
       * Virtual destructor.
       */
      virtual ~IQ();

      /**
       * Returns the IQ's type.
       * @return The IQ's type.
       */
      IqType subtype() const { return m_subtype; }

      // reimplemented from Stanza
      virtual Tag* tag() const;

    private:
#ifdef IQ_TEST
    public:
#endif
      /**
       * Creates an IQ from a tag. The original Tag will be ripped off.
       * @param tag The Tag to parse.
       */
      IQ( Tag* tag );

      void setID( const std::string& id ) { m_id = id; }

      IqType m_subtype;
  };

}

#endif // IQ_H__
