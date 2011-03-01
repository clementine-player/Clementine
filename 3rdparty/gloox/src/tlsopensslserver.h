/*
  Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef TLSOPENSSLSERVER_H__
#define TLSOPENSSLSERVER_H__

#include "tlsopensslbase.h"

#include "config.h"

#ifdef HAVE_OPENSSL

#include <openssl/ssl.h>

namespace gloox
{

  /**
   * This class implements a TLS server backend using OpenSSL.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class OpenSSLServer : public OpenSSLBase
  {
    public:
      /**
       * Constructor.
       * @param th The TLSHandler to handle TLS-related events.
       */
      OpenSSLServer( TLSHandler* th );

      /**
       * Virtual destructor.
       */
      virtual ~OpenSSLServer();

    private:
      // reimplemented from OpenSSLBase
      virtual bool privateInit();
      // reimplemented from OpenSSLBase
      virtual bool setType();

      // reimplemented from OpenSSLBase
      virtual int handshakeFunction();

  };

}

#endif // HAVE_OPENSSL

#endif // TLSOPENSSLSERVER_H__
