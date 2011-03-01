/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef TLSHANDLER_H__
#define TLSHANDLER_H__

#include "macros.h"

#include <string>

namespace gloox
{

  struct CertInfo;
  class TLSBase;

  /**
   * @brief An interface that allows for interacting with TLS implementations derived from TLSBase.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API TLSHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~TLSHandler() {}

      /**
       * Reimplement this function to receive encrypted data from a TLSBase implementation.
       * @param base The encryption implementation which called this function.
       * @param data The encrypted data (e.g. to send over the wire).
       */
      virtual void handleEncryptedData( const TLSBase* base, const std::string& data ) = 0;

      /**
       * Reimplement this function to receive decrypted data from a TLSBase implementation.
       * @param base The encryption implementation which called this function.
       * @param data The decrypted data (e.g. to parse).
       */
      virtual void handleDecryptedData( const TLSBase* base, const std::string& data ) = 0;

      /**
       * Reimplement this function to receive the result of a TLS handshake.
       * @param base The encryption implementation which called this function.
       * @param success Whether or not the handshake was successful.
       * @param certinfo Information about the server's certificate.
       */
      virtual void handleHandshakeResult( const TLSBase* base, bool success, CertInfo &certinfo ) = 0;

  };

}

#endif // TLSHANDLER_H__
