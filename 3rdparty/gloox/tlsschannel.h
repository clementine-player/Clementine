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

#ifndef TLSSCHANNEL_H__
#define TLSSCHANNEL_H__

#include "tlsbase.h"

#include "config.h"

#ifdef HAVE_WINTLS

#include <ctime>

#define SECURITY_WIN32
#include <windows.h>
#include <security.h>
#include <schnlsp.h>

namespace gloox
{

  /**
   * This class implements a TLS backend using SChannel.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class SChannel : public TLSBase
  {
    public:
      /**
       * Constructor.
       * @param th The TLSHandler to handle TLS-related events.
       * @param server The server to use in certificate verification.
       */
      SChannel( TLSHandler* th, const std::string& server );

      /**
       * Virtual destructor.
       */
      virtual ~SChannel();

      // reimplemented from TLSBase
      virtual bool init( const std::string& /*clientKey*/ = EmptyString,
                         const std::string& /*clientCerts*/ = EmptyString,
                         const StringList& /*cacerts*/ = StringList() )
        { return true; }

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

    private:
      void handshakeStage( const std::string& data );
      void setSizes();

      int filetime2int( FILETIME t );

      void validateCert();
      void connectionInfos();
      void certData();
      void setCertinfos();
      CredHandle m_credHandle;
      CtxtHandle m_context;

      SecPkgContext_StreamSizes m_sizes;

      size_t m_header_max;
      size_t m_message_max;
      size_t m_trailer_max;

      std::string m_buffer;

      bool m_cleanedup;

      // windows error outputs
//       void print_error( int errorcode, const char* place = 0 );

  };
}

#endif // HAVE_WINTLS

#endif // TLSSCHANNEL_H__
