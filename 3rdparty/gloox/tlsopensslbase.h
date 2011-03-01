/*
  Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef TLSOPENSSLBASE_H__
#define TLSOPENSSLBASE_H__

#include "tlsbase.h"

#include "config.h"

#ifdef HAVE_OPENSSL

#include <openssl/ssl.h>

namespace gloox
{

  /**
   * This is a common base class for client and server-side TLS
   * stream encryption implementations using OpenSSL.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class OpenSSLBase : public TLSBase
  {
    public:
      /**
       * Constructor.
       * @param th The TLSHandler to handle TLS-related events.
       * @param server The server to use in certificate verification.
       */
      OpenSSLBase( TLSHandler* th, const std::string& server = EmptyString );

      /**
       * Virtual destructor.
       */
      virtual ~OpenSSLBase();

      // reimplemented from TLSBase
      virtual bool init( const std::string& clientKey = EmptyString,
                         const std::string& clientCerts = EmptyString,
                         const StringList& cacerts = StringList() );

      // reimplemented from TLSBase
      virtual bool encrypt( const std::string& data );

      // reimplemented from TLSBase
      virtual int decrypt( const std::string& data );

      // reimplemented from TLSBase
      virtual void cleanup();

      // reimplemented from TLSBase
      virtual bool handshake();

      // reimplemented from TLSBase
      virtual void setCACerts( const StringList& cacerts );

      // reimplemented from TLSBase
      virtual void setClientCert( const std::string& clientKey, const std::string& clientCerts );

    protected:
      virtual bool setType() = 0;
      virtual int handshakeFunction() = 0;

      SSL* m_ssl;
      SSL_CTX* m_ctx;
      BIO* m_ibio;
      BIO* m_nbio;

    private:
      void pushFunc();
      virtual bool privateInit() { return true; }

      enum TLSOperation
      {
        TLSHandshake,
        TLSWrite,
        TLSRead
      };

      void doTLSOperation( TLSOperation op );
      int openSSLTime2UnixTime( const char* time_string );

      std::string m_recvBuffer;
      std::string m_sendBuffer;
      char* m_buf;
      const int m_bufsize;

  };

}

#endif // HAVE_OPENSSL

#endif // TLSOPENSSLBASE_H__
