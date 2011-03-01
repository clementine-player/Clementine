/*
 * Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
 * This file is part of the gloox library. http://camaya.net/gloox
 *
 * This software is distributed under a license. The full license
 * agreement can be found in the file LICENSE in this distribution.
 * This software may not be copied, modified, sold or distributed
 * other than expressed in the named license agreement.
 *
 * This software is distributed without any warranty.
 */


#ifndef TLSDEFAULT_H__
#define TLSDEFAULT_H__

#include "tlsbase.h"

namespace gloox
{

  class TLSHandler;

  /**
   * @brief This is an abstraction of the various TLS backends.
   *
   * You should use an instance of this class should you whish to use TLS encryption.
   * TLS support for the main XMPP connection is managed by Client/ClientBase directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API TLSDefault : public TLSBase
  {
    public:

      /**
       * Supported TLS types.
       */
      enum Type
      {
        VerifyingClient   =  1,     /**< TLS client, verifying, available for all
                                     * TLS implementations. */
        AnonymousClient   =  2,     /**< Anonymous TLS client (non-verifying), available with
                                     * GnuTLS. */
        VerifyingServer   =  4,     /**< TLS server, verifying, currently not available. */
        AnonymousServer   =  8      /**< Anonymous TLS server (non-verifying), available with
                                     * GnuTLS. */
      };

      /**
       * Constructs a new TLS wrapper.
       * @param th The TLSHandler to handle TLS-related events.
       * @param server The server to use in certificate verification.
       * @param type What you want to use this TLS object for.
       */
      TLSDefault( TLSHandler* th, const std::string server, Type type = VerifyingClient );

      /**
       * Virtual Destructor.
       */
      virtual ~TLSDefault();

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
      virtual bool isSecure() const;

      // reimplemented from TLSBase
      virtual void setCACerts( const StringList& cacerts );

      // reimplemented from TLSBase
      virtual const CertInfo& fetchTLSInfo() const;

      // reimplemented from TLSBase
      virtual void setClientCert( const std::string& clientKey, const std::string& clientCerts );

      /**
       * Returns an ORed list of supported TLS types.
       * @return ORed TLSDefault::type members.
       */
      static int types();

    private:
      TLSBase* m_impl;

  };
}

#endif // TLSDEFAULT_H__
