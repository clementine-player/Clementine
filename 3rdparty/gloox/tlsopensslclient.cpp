/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "tlsopensslclient.h"

#ifdef HAVE_OPENSSL

namespace gloox
{

  OpenSSLClient::OpenSSLClient( TLSHandler* th, const std::string& server )
    : OpenSSLBase( th, server )
  {
  }

  OpenSSLClient::~OpenSSLClient()
  {
  }

  bool OpenSSLClient::setType()
  {
    m_ctx = SSL_CTX_new( SSLv23_client_method() ); // FIXME: use TLSv1_client_method() as soon as OpenSSL/gtalk combo is fixed!
    if( !m_ctx )
      return false;

    return true;
  }

  int OpenSSLClient::handshakeFunction()
  {
    return SSL_connect( m_ssl );
  }

}

#endif // HAVE_OPENSSL
