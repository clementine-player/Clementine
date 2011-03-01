/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef TLSGNUTLSBASE_H__
#define TLSGNUTLSBASE_H__

#include "tlsbase.h"

#include "config.h"

#ifdef HAVE_GNUTLS

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

namespace gloox
{

  /**
   * @brief This is the common base class for (stream) encryption using GnuTLS.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GnuTLSBase : public TLSBase
  {
    public:
      /**
       * Constructor.
       * @param th The TLSHandler to handle TLS-related events.
       * @param server The server to use in certificate verification.
       */
      GnuTLSBase( TLSHandler* th, const std::string& server = EmptyString );

      /**
       * Virtual destructor.
       */
      virtual ~GnuTLSBase();

      // reimplemented from TLSBase
      virtual bool encrypt( const std::string& data );

      // reimplemented from TLSBase
      virtual int decrypt( const std::string& data );

      // reimplemented from TLSBase
      virtual void cleanup();

      // reimplemented from TLSBase
      virtual bool handshake();

      // reimplemented from TLSBase
      virtual void setCACerts( const StringList& /*cacerts*/ ) {}

      // reimplemented from TLSBase
      virtual void setClientCert( const std::string& /*clientKey*/, const std::string& /*clientCerts*/ ) {}

    protected:
      virtual void getCertInfo() {}

      gnutls_session_t* m_session;

      std::string m_recvBuffer;
      char* m_buf;
      const int m_bufsize;

      ssize_t pullFunc( void* data, size_t len );
      static ssize_t pullFunc( gnutls_transport_ptr_t ptr, void* data, size_t len );

      ssize_t pushFunc( const void* data, size_t len );
      static ssize_t pushFunc( gnutls_transport_ptr_t ptr, const void* data, size_t len );

  };

}

#endif // HAVE_GNUTLS

#endif // TLSGNUTLSBASE_H__
