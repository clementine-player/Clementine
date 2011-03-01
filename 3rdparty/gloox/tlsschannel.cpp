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

#include "tlsschannel.h"

#ifdef HAVE_WINTLS

#include <stdio.h> // just for debugging output

namespace gloox
{
  SChannel::SChannel( TLSHandler* th, const std::string& server )
    : TLSBase( th, server ), m_cleanedup( true )
  {
    //printf(">> SChannel::SChannel()\n");
  }

  SChannel::~SChannel()
  {
    m_handler = 0;
    cleanup();
    //printf(">> SChannel::~SChannel()\n");
  }

  bool SChannel::encrypt( const std::string& data )
  {
    if( !m_handler )
      return false;

    //printf(">> SChannel::encrypt()\n");
    std::string data_copy = data;

    SecBuffer buffer[4];
    SecBufferDesc buffer_desc;
    DWORD cbIoBufferLength = m_sizes.cbHeader + m_sizes.cbMaximumMessage + m_sizes.cbTrailer;

    PBYTE e_iobuffer = static_cast<PBYTE>( LocalAlloc( LMEM_FIXED, cbIoBufferLength ) );

    if( e_iobuffer == NULL )
    {
      //printf("**** Out of memory (2)\n");
      cleanup();
      if( !m_secure )
        m_handler->handleHandshakeResult( this, false, m_certInfo );
      return false;
    }
    PBYTE e_message = e_iobuffer + m_sizes.cbHeader;
    do
    {
      const size_t size = ( data_copy.size() > m_sizes.cbMaximumMessage )
                         ? m_sizes.cbMaximumMessage
                         : data_copy.size();
      memcpy( e_message, data_copy.data(), size );
      if( data_copy.size() > m_sizes.cbMaximumMessage )
        data_copy.erase( 0, m_sizes.cbMaximumMessage );
      else
        data_copy = EmptyString;

      buffer[0].pvBuffer     = e_iobuffer;
      buffer[0].cbBuffer     = m_sizes.cbHeader;
      buffer[0].BufferType   = SECBUFFER_STREAM_HEADER;

      buffer[1].pvBuffer     = e_message;
      buffer[1].cbBuffer     = size;
      buffer[1].BufferType   = SECBUFFER_DATA;

      buffer[2].pvBuffer     = static_cast<char*>(buffer[1].pvBuffer) + buffer[1].cbBuffer;
      buffer[2].cbBuffer     = m_sizes.cbTrailer;
      buffer[2].BufferType   = SECBUFFER_STREAM_TRAILER;

      buffer[3].BufferType   = SECBUFFER_EMPTY;

      buffer_desc.ulVersion       = SECBUFFER_VERSION;
      buffer_desc.cBuffers        = 4;
      buffer_desc.pBuffers        = buffer;

      SECURITY_STATUS e_status = EncryptMessage( &m_context, 0, &buffer_desc, 0 );
      if( SUCCEEDED( e_status ) )
      {
        std::string encrypted( reinterpret_cast<const char*>(e_iobuffer),
                               buffer[0].cbBuffer + buffer[1].cbBuffer + buffer[2].cbBuffer );
        m_handler->handleEncryptedData( this, encrypted );
        //if (data_copy.size() <= m_sizes.cbMaximumMessage) data_copy = EmptyString;
      }
      else
      {
        LocalFree( e_iobuffer );
        if( !m_secure )
          m_handler->handleHandshakeResult( this, false, m_certInfo );
        cleanup();
        return false;
      }
    }
    while( data_copy.size() > 0 );
    LocalFree( e_iobuffer );
    return true;
  }

  int SChannel::decrypt( const std::string& data )
  {

    if( !m_handler )
      return 0;

    //printf(">> SChannel::decrypt()\n");
    if( m_secure )
    {
      m_buffer += data;

      SecBuffer buffer[4];
      SecBufferDesc buffer_desc;
      DWORD cbIoBufferLength = m_sizes.cbHeader + m_sizes.cbMaximumMessage + m_sizes.cbTrailer;
      bool wantNewBufferSize = false;

      PBYTE e_iobuffer = static_cast<PBYTE>( LocalAlloc( LMEM_FIXED, cbIoBufferLength ) );
      if( e_iobuffer == NULL )
      {
        //printf("**** Out of memory (2)\n");
        cleanup();
        if( !m_secure )
          m_handler->handleHandshakeResult( this, false, m_certInfo );
        return 0;
      }
      SECURITY_STATUS e_status;

      do
      {
        if( wantNewBufferSize )
        {
          e_iobuffer = static_cast<PBYTE>( LocalReAlloc( e_iobuffer, cbIoBufferLength, 0 ) );
          wantNewBufferSize = false;
        }

        // copy data chunk from tmp string into encryption memory buffer
        memcpy( e_iobuffer, m_buffer.data(), m_buffer.size() >
               cbIoBufferLength ? cbIoBufferLength : m_buffer.size() );

        buffer[0].pvBuffer     = e_iobuffer;
        buffer[0].cbBuffer     = static_cast<unsigned long>( m_buffer.size() > cbIoBufferLength
                                                               ? cbIoBufferLength
                                                               : m_buffer.size() );
        buffer[0].BufferType   = SECBUFFER_DATA;
        buffer[1].cbBuffer = buffer[2].cbBuffer = buffer[3].cbBuffer = 0;
        buffer[1].BufferType = buffer[2].BufferType = buffer[3].BufferType  = SECBUFFER_EMPTY;

        buffer_desc.ulVersion       = SECBUFFER_VERSION;
        buffer_desc.cBuffers        = 4;
        buffer_desc.pBuffers        = buffer;

        unsigned long processed_data = buffer[0].cbBuffer;
        e_status = DecryptMessage( &m_context, &buffer_desc, 0, 0 );

        // print_error(e_status, "decrypt() ~ DecryptMessage()");
        // for (int n=0; n<4; n++)
        //     printf("buffer[%d].cbBuffer: %d   \t%d\n", n, buffer[n].cbBuffer, buffer[n].BufferType);

        // Locate data and (optional) extra buffers.
        SecBuffer* pDataBuffer  = NULL;
        SecBuffer* pExtraBuffer = NULL;
        for( int i = 1; i < 4; i++ )
        {
          if( pDataBuffer == NULL && buffer[i].BufferType == SECBUFFER_DATA )
          {
            pDataBuffer = &buffer[i];
            //printf("buffer[%d].BufferType = SECBUFFER_DATA\n",i);
          }
          if( pExtraBuffer == NULL && buffer[i].BufferType == SECBUFFER_EXTRA )
          {
            pExtraBuffer = &buffer[i];
          }
        }
        if( e_status == SEC_E_OK )
        {
          std::string decrypted( reinterpret_cast<const char*>( pDataBuffer->pvBuffer ),
                                 pDataBuffer->cbBuffer );
          m_handler->handleDecryptedData( this, decrypted );
          if( pExtraBuffer == NULL )
          {
            m_buffer.erase( 0, processed_data );
          }
          else
          {
            //std::cout << "m_buffer.size() = " << pExtraBuffer->cbBuffer << std::endl;
            m_buffer.erase( 0, processed_data - pExtraBuffer->cbBuffer );
            //std::cout << "m_buffer.size() = " << m_buffer.size() << std::endl;

            cbIoBufferLength = m_sizes.cbHeader + m_sizes.cbMaximumMessage + m_sizes.cbTrailer;
            wantNewBufferSize = true;
          }
        }
        else if( e_status == SEC_E_INCOMPLETE_MESSAGE )
        {
          if( cbIoBufferLength < 200000 && m_buffer.size() > cbIoBufferLength )
          {
            cbIoBufferLength += 1000;
            wantNewBufferSize = true;
          }
          else
          {
            cbIoBufferLength = m_sizes.cbHeader + m_sizes.cbMaximumMessage + m_sizes.cbTrailer;
            wantNewBufferSize = true;
            break;
          }
        }
        else
        {
          //std::cout << "decrypt !!!ERROR!!!\n";
          if( !m_secure )
            m_handler->handleHandshakeResult( this, false, m_certInfo );
          cleanup();
          break;
        }
      }
      while( m_buffer.size() != 0 );
      LocalFree( e_iobuffer );
    }
    else
    {
      handshakeStage( data );
    }
    //printf("<< SChannel::decrypt()\n");
    return 0;
  }

  void SChannel::cleanup()
  {
    if( !m_mutex.trylock() )
      return;

    m_buffer = "";
    if( !m_cleanedup )
    {
      m_valid = false;
      m_secure = false;
      m_cleanedup = true;
      DeleteSecurityContext( &m_context );
      FreeCredentialsHandle( &m_credHandle );
    }

    m_mutex.unlock();
  }

  bool SChannel::handshake()
  {
    if( !m_handler )
      return false;

    //printf(">> SChannel::handshake()\n");
    SECURITY_STATUS error;
    ULONG return_flags;
    TimeStamp t;
    SecBuffer obuf[1];
    SecBufferDesc obufs;
    SCHANNEL_CRED tlscred;
    ULONG request = ISC_REQ_ALLOCATE_MEMORY
                    | ISC_REQ_CONFIDENTIALITY
                    | ISC_REQ_EXTENDED_ERROR
                    | ISC_REQ_INTEGRITY
                    | ISC_REQ_REPLAY_DETECT
                    | ISC_REQ_SEQUENCE_DETECT
                    | ISC_REQ_STREAM
                    | ISC_REQ_MANUAL_CRED_VALIDATION;

    /* initialize TLS credential */
    memset( &tlscred, 0, sizeof( SCHANNEL_CRED ) );
    tlscred.dwVersion = SCHANNEL_CRED_VERSION;
    tlscred.grbitEnabledProtocols = SP_PROT_TLS1;
    /* acquire credentials */
    error = AcquireCredentialsHandle( 0,
                                      UNISP_NAME,
                                      SECPKG_CRED_OUTBOUND,
                                      0,
                                      &tlscred,
                                      0,
                                      0,
                                      &m_credHandle,
                                      &t );
    //print_error(error, "handshake() ~ AcquireCredentialsHandle()");
    if( error != SEC_E_OK )
    {
      cleanup();
      m_handler->handleHandshakeResult( this, false, m_certInfo );
      return false;
    }
    else
    {
      /* initialize buffers */
      obuf[0].cbBuffer = 0;
      obuf[0].pvBuffer = 0;
      obuf[0].BufferType = SECBUFFER_TOKEN;
      /* initialize buffer descriptors */
      obufs.ulVersion = SECBUFFER_VERSION;
      obufs.cBuffers = 1;
      obufs.pBuffers = obuf;
      /* negotiate security */
      SEC_CHAR* hname = const_cast<char*>( m_server.c_str() );

      error = InitializeSecurityContextA( &m_credHandle,
                                         0,
                                         hname,
                                         request,
                                         0,
                                         SECURITY_NETWORK_DREP,
                                         0,
                                         0,
                                         &m_context,
                                         &obufs,
                                         &return_flags,
                                         NULL );
      //print_error(error, "handshake() ~ InitializeSecurityContext()");

      if( error == SEC_I_CONTINUE_NEEDED )
      {
        m_cleanedup = false;
        //std::cout << "obuf[1].cbBuffer: " << obuf[0].cbBuffer << "\n";
        std::string senddata( static_cast<char*>(obuf[0].pvBuffer), obuf[0].cbBuffer );
        FreeContextBuffer( obuf[0].pvBuffer );
        m_handler->handleEncryptedData( this, senddata );
        return true;
      }
      else
      {
        cleanup();
        m_handler->handleHandshakeResult( this, false, m_certInfo );
        return false;
      }
    }
  }

  void SChannel::handshakeStage( const std::string& data )
  {
    //printf(" >> handshake_stage\n");
    m_buffer += data;

    SECURITY_STATUS error;
    ULONG a;
    TimeStamp t;
    SecBuffer ibuf[2], obuf[1];
    SecBufferDesc ibufs, obufs;
    ULONG request = ISC_REQ_ALLOCATE_MEMORY
                    | ISC_REQ_CONFIDENTIALITY
                    | ISC_REQ_EXTENDED_ERROR
                    | ISC_REQ_INTEGRITY
                    | ISC_REQ_REPLAY_DETECT
                    | ISC_REQ_SEQUENCE_DETECT
                    | ISC_REQ_STREAM
                    | ISC_REQ_MANUAL_CRED_VALIDATION;

    SEC_CHAR* hname = const_cast<char*>( m_server.c_str() );

    do
    {
      /* initialize buffers */
      ibuf[0].cbBuffer = static_cast<unsigned long>( m_buffer.size() );
      ibuf[0].pvBuffer = static_cast<void*>( const_cast<char*>( m_buffer.c_str() ) );
      //std::cout << "Size: " << m_buffer.size() << "\n";
      ibuf[1].cbBuffer = 0;
      ibuf[1].pvBuffer = 0;
      obuf[0].cbBuffer = 0;
      obuf[0].pvBuffer = 0;

      ibuf[0].BufferType = SECBUFFER_TOKEN;
      ibuf[1].BufferType = SECBUFFER_EMPTY;
      obuf[0].BufferType = SECBUFFER_EMPTY;
      /* initialize buffer descriptors */
      ibufs.ulVersion = obufs.ulVersion = SECBUFFER_VERSION;
      ibufs.cBuffers = 2;
      obufs.cBuffers = 1;
      ibufs.pBuffers = ibuf;
      obufs.pBuffers = obuf;

      /*
       * std::cout << "obuf[0].cbBuffer: " << obuf[0].cbBuffer << "\t" << obuf[0].BufferType << "\n";
       * std::cout << "ibuf[0].cbBuffer: " << ibuf[0].cbBuffer << "\t" << ibuf[0].BufferType << "\n";
       * std::cout << "ibuf[1].cbBuffer: " << ibuf[1].cbBuffer << "\t" << ibuf[1].BufferType << "\n";
       */

      /* negotiate security */
      error = InitializeSecurityContextA( &m_credHandle,
                                         &m_context,
                                         hname,
                                         request,
                                         0,
                                         0,
                                         &ibufs,
                                         0,
                                         0,
                                         &obufs,
                                         &a,
                                         &t );
      //print_error(error, "handshake() ~ InitializeSecurityContext()");
      if( error == SEC_E_OK )
      {
        // EXTRA STUFF??
        if( ibuf[1].BufferType == SECBUFFER_EXTRA )
        {
          m_buffer.erase( 0, m_buffer.size() - ibuf[1].cbBuffer );
        }
        else
        {
          m_buffer = EmptyString;
        }
        setSizes();
        setCertinfos();

        m_secure = true;
        m_handler->handleHandshakeResult( this, true, m_certInfo );
        break;
      }
      else if( error == SEC_I_CONTINUE_NEEDED )
      {
        /*
         * std::cout << "obuf[0].cbBuffer: " << obuf[0].cbBuffer << "\t" << obuf[0].BufferType << "\n";
         * std::cout << "ibuf[0].cbBuffer: " << ibuf[0].cbBuffer << "\t" << ibuf[0].BufferType << "\n";
         * std::cout << "ibuf[1].cbBuffer: " << ibuf[1].cbBuffer << "\t" << ibuf[1].BufferType << "\n";
         */

        // STUFF TO SEND??
        if( obuf[0].cbBuffer != 0 && obuf[0].pvBuffer != NULL )
        {
          std::string senddata( static_cast<char*>(obuf[0].pvBuffer), obuf[0].cbBuffer );
          FreeContextBuffer( obuf[0].pvBuffer );
          m_handler->handleEncryptedData( this, senddata );
        }
        // EXTRA STUFF??
        if( ibuf[1].BufferType == SECBUFFER_EXTRA )
        {
          m_buffer.erase( 0, m_buffer.size() - ibuf[1].cbBuffer );
          // Call again if we aren't sending anything (otherwise the server will not send anything back
          // and this function won't get called again to finish the processing).  This is needed for
          // NT4.0 which does not seem to process the entire buffer the first time around
          if( obuf[0].cbBuffer == 0 )
            handshakeStage( EmptyString );
        }
        else
        {
          m_buffer = EmptyString;
        }
        return;
      }
      else if( error == SEC_I_INCOMPLETE_CREDENTIALS )
      {
        handshakeStage( EmptyString );
      }
      else if( error == SEC_E_INCOMPLETE_MESSAGE )
      {
        break;
      }
      else
      {
        cleanup();
        m_handler->handleHandshakeResult( this, false, m_certInfo );
        break;
      }
    }
    while( true );
  }

  void SChannel::setCACerts( const StringList& /*cacerts*/ ) {}

  void SChannel::setClientCert( const std::string& /*clientKey*/, const std::string& /*clientCerts*/ ) {}

  void SChannel::setSizes()
  {
    if( QueryContextAttributes( &m_context, SECPKG_ATTR_STREAM_SIZES, &m_sizes ) == SEC_E_OK )
    {
      //std::cout << "set_sizes success\n";
    }
    else
    {
      //std::cout << "set_sizes no success\n";
      cleanup();
      m_handler->handleHandshakeResult( this, false, m_certInfo );
    }
  }

  int SChannel::filetime2int( FILETIME t )
  {
    SYSTEMTIME stUTC;
    FileTimeToSystemTime(&t, &stUTC);
    std::tm ts;
    ts.tm_year = stUTC.wYear - 1900;
    ts.tm_mon = stUTC.wMonth - 1;
    ts.tm_mday = stUTC.wDay;
    ts.tm_hour = stUTC.wHour;
    ts.tm_min = stUTC.wMinute;
    ts.tm_sec = stUTC.wSecond;

    time_t unixtime;
    if ( (unixtime = mktime(&ts)) == -1 )
      unixtime = 0;
    return (int)unixtime;
  }

  void SChannel::validateCert()
  {
    bool valid = false;
    HTTPSPolicyCallbackData policyHTTPS;
    CERT_CHAIN_POLICY_PARA policyParameter;
    CERT_CHAIN_POLICY_STATUS policyStatus;

    PCCERT_CONTEXT remoteCertContext = NULL;
    PCCERT_CHAIN_CONTEXT chainContext = NULL;
    CERT_CHAIN_PARA chainParameter;
    PSTR serverName = const_cast<char*>( m_server.c_str() );

    PWSTR uServerName = NULL;
    DWORD csizeServerName;

    LPSTR Usages[] = {
      szOID_PKIX_KP_SERVER_AUTH,
      szOID_SERVER_GATED_CRYPTO,
      szOID_SGC_NETSCAPE
    };
    DWORD cUsages = sizeof( Usages ) / sizeof( LPSTR );

    do
    {
      // Get server's certificate.
      if( QueryContextAttributes( &m_context, SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                  (PVOID)&remoteCertContext ) != SEC_E_OK )
      {
        //printf("Error querying remote certificate\n");
        // !!! THROW SOME ERROR
        break;
      }

      // unicode conversation
      // calculating unicode server name size
      csizeServerName = MultiByteToWideChar( CP_ACP, 0, serverName, -1, NULL, 0 );
      uServerName = reinterpret_cast<WCHAR *>( LocalAlloc( LMEM_FIXED,
                                                             csizeServerName * sizeof( WCHAR ) ) );
      if( uServerName == NULL )
      {
        //printf("SEC_E_INSUFFICIENT_MEMORY ~ Not enough memory!!!\n");
        break;
      }

      // convert into unicode
      csizeServerName = MultiByteToWideChar( CP_ACP, 0, serverName, -1, uServerName, csizeServerName );
      if( csizeServerName == 0 )
      {
        //printf("SEC_E_WRONG_PRINCIPAL\n");
        break;
      }

      // create the chain
      ZeroMemory( &chainParameter, sizeof( chainParameter ) );
      chainParameter.cbSize = sizeof( chainParameter );
      chainParameter.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
      chainParameter.RequestedUsage.Usage.cUsageIdentifier     = cUsages;
      chainParameter.RequestedUsage.Usage.rgpszUsageIdentifier = Usages;

      if( !CertGetCertificateChain( NULL, remoteCertContext, NULL, remoteCertContext->hCertStore,
                                    &chainParameter, 0, NULL, &chainContext ) )
      {
//         DWORD status = GetLastError();
//         printf("Error 0x%x returned by CertGetCertificateChain!!!\n", status);
        break;
      }

      // validate the chain
      ZeroMemory( &policyHTTPS, sizeof( HTTPSPolicyCallbackData ) );
      policyHTTPS.cbStruct           = sizeof( HTTPSPolicyCallbackData );
      policyHTTPS.dwAuthType         = AUTHTYPE_SERVER;
      policyHTTPS.fdwChecks          = 0;
      policyHTTPS.pwszServerName     = uServerName;

      memset( &policyParameter, 0, sizeof( policyParameter ) );
      policyParameter.cbSize            = sizeof( policyParameter );
      policyParameter.pvExtraPolicyPara = &policyHTTPS;

      memset( &policyStatus, 0, sizeof( policyStatus ) );
      policyStatus.cbSize = sizeof( policyStatus );

      if( !CertVerifyCertificateChainPolicy( CERT_CHAIN_POLICY_SSL, chainContext, &policyParameter,
                                             &policyStatus ) )
      {
//         DWORD status = GetLastError();
//         printf("Error 0x%x returned by CertVerifyCertificateChainPolicy!!!\n", status);
        break;
      }

      if( policyStatus.dwError )
      {
        //printf("Trust Error!!!}n");
        break;
      }
      valid = true;
    }
    while( false );
    // cleanup
    if( chainContext ) CertFreeCertificateChain( chainContext );
    m_certInfo.chain = valid;
  }

  void SChannel::connectionInfos()
  {
    SecPkgContext_ConnectionInfo conn_info;

    memset( &conn_info, 0, sizeof( conn_info ) );

    if( QueryContextAttributes( &m_context, SECPKG_ATTR_CONNECTION_INFO, &conn_info ) == SEC_E_OK )
    {
      switch( conn_info.dwProtocol )
      {
        case SP_PROT_TLS1_CLIENT:
          m_certInfo.protocol = "TLSv1";
          break;
        case SP_PROT_SSL3_CLIENT:
          m_certInfo.protocol = "SSLv3";
          break;
        default:
          m_certInfo.protocol = "unknown";
      }

      switch( conn_info.aiCipher )
      {
        case CALG_3DES:
          m_certInfo.cipher = "3DES";
          break;
        case CALG_AES_128:
          m_certInfo.cipher = "AES_128";
          break;
        case CALG_AES_256:
          m_certInfo.cipher = "AES_256";
          break;
        case CALG_DES:
          m_certInfo.cipher = "DES";
          break;
        case CALG_RC2:
          m_certInfo.cipher = "RC2";
          break;
        case CALG_RC4:
          m_certInfo.cipher = "RC4";
          break;
        default:
          m_certInfo.cipher = EmptyString;
      }

      switch( conn_info.aiHash )
      {
        case CALG_MD5:
          m_certInfo.mac = "MD5";
          break;
        case CALG_SHA:
          m_certInfo.mac = "SHA";
          break;
        default:
          m_certInfo.mac = EmptyString;
      }
    }
  }

  void SChannel::certData()
  {
    PCCERT_CONTEXT remoteCertContext = NULL;
    CHAR certString[1000];

    // getting server's certificate
    if( QueryContextAttributes( &m_context, SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                (PVOID)&remoteCertContext ) != SEC_E_OK )
    {
      return;
    }

    // setting certificat's lifespan
    m_certInfo.date_from = filetime2int( remoteCertContext->pCertInfo->NotBefore );
    m_certInfo.date_to = filetime2int( remoteCertContext->pCertInfo->NotAfter );

    if( !CertNameToStrA( remoteCertContext->dwCertEncodingType,
                        &remoteCertContext->pCertInfo->Subject,
                        CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                        certString, sizeof( certString ) ) )
    {
      return;
    }
    m_certInfo.server = certString;

    if( !CertNameToStrA( remoteCertContext->dwCertEncodingType,
                       &remoteCertContext->pCertInfo->Issuer,
                       CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                       certString, sizeof( certString ) ) )
    {
      return;
    }
    m_certInfo.issuer = certString;
  }

  void SChannel::setCertinfos()
  {
    validateCert();
    connectionInfos();
    certData();
  }

#if 0
  void SChannel::print_error( int errorcode, const char* place )
  {
    printf( "Win error at %s.\n", place );
    switch( errorcode )
    {
      case SEC_E_OK:
        printf( "\tValue:\tSEC_E_OK\n" );
        printf( "\tDesc:\tNot really an error. Everything is fine.\n" );
        break;
      case SEC_E_INSUFFICIENT_MEMORY:
        printf( "\tValue:\tSEC_E_INSUFFICIENT_MEMORY\n" );
        printf( "\tDesc:\tThere is not enough memory available to complete the requested action.\n" );
        break;
      case SEC_E_INTERNAL_ERROR:
        printf( "\tValue:\tSEC_E_INTERNAL_ERROR\n" );
        printf( "\tDesc:\tAn error occurred that did not map to an SSPI error code.\n" );
        break;
      case SEC_E_NO_CREDENTIALS:
        printf( "\tValue:\tSEC_E_NO_CREDENTIALS\n" );
        printf( "\tDesc:\tNo credentials are available in the security package.\n" );
        break;
      case SEC_E_NOT_OWNER:
        printf( "\tValue:\tSEC_E_NOT_OWNER\n" );
        printf( "\tDesc:\tThe caller of the function does not have the necessary credentials.\n" );
        break;
      case SEC_E_SECPKG_NOT_FOUND:
        printf( "\tValue:\tSEC_E_SECPKG_NOT_FOUND\n" );
        printf( "\tDesc:\tThe requested security package does not exist. \n" );
        break;
      case SEC_E_UNKNOWN_CREDENTIALS:
        printf( "\tValue:\tSEC_E_UNKNOWN_CREDENTIALS\n" );
        printf( "\tDesc:\tThe credentials supplied to the package were not recognized.\n" );
        break;
      case SEC_E_INCOMPLETE_MESSAGE:
        printf( "\tValue:\tSEC_E_INCOMPLETE_MESSAGE\n" );
        printf( "\tDesc:\tData for the whole message was not read from the wire.\n" );
        break;
      case SEC_E_INVALID_HANDLE:
        printf( "\tValue:\tSEC_E_INVALID_HANDLE\n" );
        printf( "\tDesc:\tThe handle passed to the function is invalid.\n" );
        break;
      case SEC_E_INVALID_TOKEN:
        printf( "\tValue:\tSEC_E_INVALID_TOKEN\n" );
        printf( "\tDesc:\tThe error is due to a malformed input token, such as a token "
                "corrupted in transit...\n" );
        break;
      case SEC_E_LOGON_DENIED:
        printf( "\tValue:\tSEC_E_LOGON_DENIED\n" );
        printf( "\tDesc:\tThe logon failed.\n" );
        break;
      case SEC_E_NO_AUTHENTICATING_AUTHORITY:
        printf( "\tValue:\tSEC_E_NO_AUTHENTICATING_AUTHORITY\n" );
        printf( "\tDesc:\tNo authority could be contacted for authentication...\n" );
        break;
      case SEC_E_TARGET_UNKNOWN:
        printf( "\tValue:\tSEC_E_TARGET_UNKNOWN\n" );
        printf( "\tDesc:\tThe target was not recognized.\n" );
        break;
      case SEC_E_UNSUPPORTED_FUNCTION:
        printf( "\tValue:\tSEC_E_UNSUPPORTED_FUNCTION\n" );
        printf( "\tDesc:\tAn invalid context attribute flag (ISC_REQ_DELEGATE or "
                "ISC_REQ_PROMPT_FOR_CREDS)...\n" );
        break;
      case SEC_E_WRONG_PRINCIPAL:
        printf( "\tValue:\tSEC_E_WRONG_PRINCIPAL\n" );
        printf( "\tDesc:\tThe principal that received the authentication request "
                "is not the same as the...\n" );
        break;
      case SEC_I_COMPLETE_AND_CONTINUE:
        printf( "\tValue:\tSEC_I_COMPLETE_AND_CONTINUE\n" );
        printf( "\tDesc:\tThe client must call CompleteAuthToken and then pass the output...\n" );
        break;
      case SEC_I_COMPLETE_NEEDED:
        printf( "\tValue:\tSEC_I_COMPLETE_NEEDED\n" );
        printf( "\tDesc:\tThe client must finish building the message and then "
                "call the CompleteAuthToken function.\n" );
        break;
      case SEC_I_CONTINUE_NEEDED:
        printf( "\tValue:\tSEC_I_CONTINUE_NEEDED\n" );
        printf( "\tDesc:\tThe client must send the output token to the server "
                "and wait for a return token...\n" );
        break;
      case SEC_I_INCOMPLETE_CREDENTIALS:
        printf( "\tValue:\tSEC_I_INCOMPLETE_CREDENTIALS\n" );
        printf( "\tDesc:\tThe server has requested client authentication, "
                "and the supplied credentials either...\n" );
        break;
      default:
        printf( "\tValue:\t%d\n", errorcode );
        printf( "\tDesc:\tUnknown error code.\n" );
    }
  }
#endif

}

#endif // HAVE_WINTLS
