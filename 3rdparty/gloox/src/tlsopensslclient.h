/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef TLSOPENSSLCLIENT_H__
#define TLSOPENSSLCLIENT_H__

#include "tlsopensslbase.h"

#include "config.h"

#ifdef HAVE_OPENSSL

#include <openssl/ssl.h>

namespace gloox
{

  /**
   * This class implements a TLS client backend using OpenSSL.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class OpenSSLClient : public OpenSSLBase
  {
    public:
      /**
       * Constructor.
       * @param th The TLSHandler to handle TLS-related events.
       * @param server The server to use in certificate verification.
       */
      OpenSSLClient( TLSHandler* th, const std::string& server );

      /**
       * Virtual destructor.
       */
      virtual ~OpenSSLClient();

    private:
      // reimplemented from OpenSSLBase
      virtual bool setType();

      // reimplemented from OpenSSLBase
      virtual int handshakeFunction();

  };

}

#endif // HAVE_OPENSSL

#endif // TLSOPENSSLCLIENT_H__
