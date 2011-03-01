/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef TLSGNUTLSCLIENTANON_H__
#define TLSGNUTLSCLIENTANON_H__

#include "tlsgnutlsbase.h"

#include "config.h"

#ifdef HAVE_GNUTLS

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

namespace gloox
{

  /**
   * @brief This class implements an anonymous TLS backend using GnuTLS.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GnuTLSClientAnon : public GnuTLSBase
  {
    public:
      /**
       * Constructor.
       * @param th The TLSHandler to handle TLS-related events.
       */
      GnuTLSClientAnon( TLSHandler* th );

      /**
       * Virtual destructor.
       */
      virtual ~GnuTLSClientAnon();

      // reimplemented from TLSBase
      virtual bool init( const std::string& clientKey = EmptyString,
                         const std::string& clientCerts = EmptyString,
                         const StringList& cacerts = StringList() );

      // reimplemented from TLSBase
      virtual void cleanup();

    private:
      virtual void getCertInfo();

      gnutls_anon_client_credentials_t m_anoncred;
  };

}

#endif // HAVE_GNUTLS

#endif // TLSGNUTLSCLIENTANON_H__
