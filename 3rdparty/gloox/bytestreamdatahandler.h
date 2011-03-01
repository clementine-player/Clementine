/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef BYTESTREAMDATAHANDLER_H__
#define BYTESTREAMDATAHANDLER_H__

#include "macros.h"

#include <string>

namespace gloox
{

  class Bytestream;
  class IQ;

  /**
   * @brief A virtual interface that allows implementors to receive data
   * sent over a SOCKS5 Bytestream as defined in XEP-0066, or an In-Band Bytestream
   * as defined in XEP-0047. You'll also need it for sending of data.
   *
   * An BytestreamDataHandler is registered with a Bytestream.
   *
   * See SIProfileFT for more information regarding file transfer.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API BytestreamDataHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~BytestreamDataHandler() {}

      /**
       * Reimplement this function to receive data which is sent over the bytestream.
       * The data received here is (probably) only a single chunk of the complete data (depending
       * on the amount of data you want to send).
       * @param bs The bytestream.
       * @param data The actual stream payload.
       */
      virtual void handleBytestreamData( Bytestream* bs, const std::string& data ) = 0;

      /**
       * Notifies about an error occuring while using a bytestream.
       * When this handler is called the stream has already been closed.
       * @param bs The bytestream.
       * @param iq The error stanza.
       */
      virtual void handleBytestreamError( Bytestream* bs, const IQ& iq ) = 0;

      /**
       * Notifies the handler that the given bytestream has been acknowledged
       * and is ready to send/receive data.
       * @param bs The opened bytestream.
       */
      virtual void handleBytestreamOpen( Bytestream* bs ) = 0;

      /**
       * Notifies the handler that the given bytestream has been closed.
       * @param bs The closed bytestream.
       */
      virtual void handleBytestreamClose( Bytestream* bs ) = 0;

  };

}

#endif // BYTESTREAMDATAHANDLER_H__
