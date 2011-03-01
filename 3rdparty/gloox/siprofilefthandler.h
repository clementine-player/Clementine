/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIPROFILEFTHANDLER_H__
#define SIPROFILEFTHANDLER_H__

#include "jid.h"

#include <string>

namespace gloox
{

  class JID;
  class IQ;
  class Bytestream;

  /**
   * @brief An abstract base class to handle file transfer (FT) requests.
   *
   * See SIProfileFT for more information regarding file transfer.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIProfileFTHandler
  {

    public:
      /**
       * Virtual destructor.
       */
      virtual ~SIProfileFTHandler() {}

      /**
       * This function is called to handle incoming file transfer requests, i.e. a remote entity requested
       * to send a file to you. You should use either SIProfileFT::acceptFT() or
       * SIProfileFT::declineFT() to accept or reject the request, respectively.
       * @param from The file transfer requestor.
       * @param to The file transfer recipient. Usuall oneself. Used in component scenario.
       * @param sid The requested stream's ID. This sid MUST be supplied to SIProfileFT::acceptFT()
       * and SIProfileFT::declineFT(), respectively.
       * @param name The file name.
       * @param size The file size.
       * @param hash The file content's MD5 sum.
       * @param date The file's last modification time.
       * @param mimetype The file's mime-type.
       * @param desc The file's description.
       * @param stypes An ORed list of @link gloox::SIProfileFT::StreamType SIProfileFT::StreamType @endlink
       * indicating the StreamTypes the initiator supports.
       */
      virtual void handleFTRequest( const JID& from, const JID& to, const std::string& sid,
                                    const std::string& name, long size, const std::string& hash,
                                    const std::string& date, const std::string& mimetype,
                                    const std::string& desc, int stypes ) = 0;

      /**
       * This function is called to handle a request error or decline.
       * @param iq The complete error stanza.
       * @param sid The request's SID.
       */
      virtual void handleFTRequestError( const IQ& iq, const std::string& sid ) = 0;

      /**
       * This function is called to pass a negotiated bytestream (SOCKS5 or IBB).
       * The bytestream is not yet open and not ready to send/receive data.
       * @note To initialize the bytestream and to prepare it for data transfer
       * do the following, preferable in that order:
       * @li register a BytestreamDataHandler with the Bytestream,
       * @li set up a separate thread for the bytestream or integrate it into
       * your main loop,
       * @li call its connect() method and check the return value.
       * To not block your application while the data transfer and/or the connection
       * attempts last, you most likely want to put the bytestream into its own
       * thread or process (before calling connect() on it). It is safe to do so
       * without additional synchronization.
       * @param bs The bytestream.
       */
      virtual void handleFTBytestream( Bytestream* bs ) = 0;

      /**
       * This function is called if the contact chose OOB as the mechanism.
       * @param from The remote contact's JID.
       * @param to The local recipient's JID. Usually oneself. Used in component scenario.
       * @param sid The stream's ID.
       * @return The file's URL.
       */
      virtual const std::string handleOOBRequestResult( const JID& from, const JID& to, const std::string& sid ) = 0;

  };

}

#endif // SIPROFILEFTHANDLER_H__
