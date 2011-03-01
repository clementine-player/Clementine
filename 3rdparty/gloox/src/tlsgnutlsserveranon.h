/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef TLSGNUTLSSERVERANON_H__
#define TLSGNUTLSSERVERANON_H__

#include "tlsgnutlsbase.h"

#include "config.h"

#ifdef HAVE_GNUTLS

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

namespace gloox
{

  /**
   * @brief This class implements (stream) encryption using GnuTLS server-side.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GnuTLSServerAnon : public GnuTLSBase
  {
    public:
      /**
       * Constructor.
       * @param th The TLSHandler to handle TLS-related events.
       */
      GnuTLSServerAnon( TLSHandler* th );

      /**
       * Virtual destructor.
       */
      virtual ~GnuTLSServerAnon();

      // reimplemented from TLSBase
      virtual bool init( const std::string& clientKey = EmptyString,
                         const std::string& clientCerts = EmptyString,
                         const StringList& cacerts = StringList() );

      // reimplemented from TLSBase
      virtual void cleanup();

    private:
      virtual void getCertInfo();
      void generateDH();

      gnutls_anon_server_credentials_t m_anoncred;
      gnutls_dh_params_t m_dhParams;

      const int m_dhBits;

  };

}

#endif // HAVE_GNUTLS

#endif // TLSGNUTLSSERVERANON_H__
