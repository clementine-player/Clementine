/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef BYTESTREAMHANDLER_H__
#define BYTESTREAMHANDLER_H__

#include "macros.h"
#include "jid.h"
#include "bytestream.h"
#include "iq.h"

namespace gloox
{

  /**
   * @brief A virtual interface that allows to receive new incoming Bytestream requests
   * from remote entities.
   *
   * You should not need to use this interface directly.
   *
   * See SIProfileFT on how to implement file transfer in general.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API BytestreamHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~BytestreamHandler() {}

      /**
       * Notifies the implementor of a new incoming bytestream request.
       * You have to call either
       * BytestreamManager::acceptBytestream() or
       * BytestreamManager::rejectBytestream(), to accept or reject the bytestream
       * request, respectively.
       * @param sid The bytestream's id, to be passed to BytestreamManager::acceptBytestream()
       * and BytestreamManager::rejectBytestream(), respectively.
       * @param from The remote initiator of the bytestream request.
       */
      virtual void handleIncomingBytestreamRequest( const std::string& sid, const JID& from ) = 0;

      /**
       * Notifies the implementor of a new incoming bytestream. The bytestream is not yet ready to
       * send data.
       * To initialize the bytestream and to prepare it for data transfer, register a
       * BytestreamDataHandler with it and call its connect() method.
       * To not block your application while the data transfer lasts, you most
       * likely want to put the bytestream into its own thread or process (before calling connect() on it).
       * It is safe to do so without additional synchronization.
       * When you are finished using the bytestream, use SIProfileFT::dispose() to get rid of it.
       * @param bs The bytestream.
       */
      virtual void handleIncomingBytestream( Bytestream* bs ) = 0;

      /**
       * Notifies the implementor of successful establishing of an outgoing bytestream request.
       * The stream has been accepted by the remote entity and is ready to send data.
       * The BytestreamHandler does @b not become the owner of the Bytestream object.
       * Use SIProfileFT::dispose() to get rid of the bytestream object after it has been closed.
       * @param bs The new bytestream.
       */
      virtual void handleOutgoingBytestream( Bytestream* bs ) = 0;

      /**
       * Notifies the handler of errors occuring when a bytestream was requested.
       * For example, if the remote entity does not implement SOCKS5 bytestreams.
       * @param iq The error stanza.
       * @param sid The request's SID.
       */
      virtual void handleBytestreamError( const IQ& iq, const std::string& sid ) = 0;

  };

}

#endif // BYTESTREAMHANDLER_H__
