/*
  Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "tlsopensslbase.h"

#ifdef HAVE_OPENSSL

#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstdlib>

#include <openssl/err.h>

namespace gloox
{

  OpenSSLBase::OpenSSLBase( TLSHandler* th, const std::string& server )
    : TLSBase( th, server ), m_ssl( 0 ), m_ctx( 0 ), m_buf( 0 ), m_bufsize( 17000 )
  {
    m_buf = (char*)calloc( m_bufsize + 1, sizeof( char ) );
  }

  OpenSSLBase::~OpenSSLBase()
  {
    m_handler = 0;
    free( m_buf );
    SSL_CTX_free( m_ctx );
    SSL_shutdown( m_ssl );
    SSL_free( m_ssl );
    BIO_free( m_nbio );
    cleanup();
  }

  bool OpenSSLBase::init( const std::string& clientKey,
                          const std::string& clientCerts,
                          const StringList& cacerts )
  {
    if( m_initLib )
      SSL_library_init();

    SSL_COMP_add_compression_method( 193, COMP_zlib() );

    OpenSSL_add_all_algorithms();

    if( !setType() ) //inits m_ctx
      return false;

    setClientCert( clientKey, clientCerts );
    setCACerts( cacerts );

    if( !SSL_CTX_set_cipher_list( m_ctx, "HIGH:MEDIUM:AES:@STRENGTH" ) )
      return false;

    m_ssl = SSL_new( m_ctx );
    if( !m_ssl )
      return false;

    if( !BIO_new_bio_pair( &m_ibio, 0, &m_nbio, 0 ) )
      return false;

    SSL_set_bio( m_ssl, m_ibio, m_ibio );
    SSL_set_mode( m_ssl, SSL_MODE_AUTO_RETRY | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_ENABLE_PARTIAL_WRITE );

    ERR_load_crypto_strings();
    SSL_load_error_strings();

    if( !privateInit() )
      return false;

    m_valid = true;
    return true;
  }

  bool OpenSSLBase::encrypt( const std::string& data )
  {
    m_sendBuffer += data;

    if( !m_secure )
    {
      handshake();
      return 0;
    }

    doTLSOperation( TLSWrite );
    return true;
  }

  int OpenSSLBase::decrypt( const std::string& data )
  {
    m_recvBuffer += data;

    if( !m_secure )
    {
      handshake();
      return 0;
    }

    doTLSOperation( TLSRead );
    return true;
  }

  void OpenSSLBase::setCACerts( const StringList& cacerts )
  {
    m_cacerts = cacerts;

    StringList::const_iterator it = m_cacerts.begin();
    for( ; it != m_cacerts.end(); ++it )
      SSL_CTX_load_verify_locations( m_ctx, (*it).c_str(), 0 );
  }

  void OpenSSLBase::setClientCert( const std::string& clientKey, const std::string& clientCerts )
  {
    m_clientKey = clientKey;
    m_clientCerts = clientCerts;

    if( !m_clientKey.empty() && !m_clientCerts.empty() )
    {
      if( SSL_CTX_use_certificate_chain_file( m_ctx, m_clientCerts.c_str() ) != 1 )
      {
        // FIXME
      }
      if( SSL_CTX_use_RSAPrivateKey_file( m_ctx, m_clientKey.c_str(), SSL_FILETYPE_PEM ) != 1 )
      {
        // FIXME
      }
    }

    if ( SSL_CTX_check_private_key( m_ctx ) != 1 )
    {
        // FIXME
    }
  }

  void OpenSSLBase::cleanup()
  {
    if( !m_mutex.trylock() )
      return;

    m_secure = false;
    m_valid = false;

    m_mutex.unlock();
  }

  void OpenSSLBase::doTLSOperation( TLSOperation op )
  {
    if( !m_handler )
      return;

    int ret = 0;
    bool onceAgain = false;

    do
    {
      switch( op )
      {
        case TLSHandshake:
          ret = handshakeFunction();
          break;
        case TLSWrite:
          ret = SSL_write( m_ssl, m_sendBuffer.c_str(), m_sendBuffer.length() );
          break;
        case TLSRead:
          ret = SSL_read( m_ssl, m_buf, m_bufsize );
          break;
      }

      switch( SSL_get_error( m_ssl, ret ) )
      {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          pushFunc();
          break;
        case SSL_ERROR_NONE:
          if( op == TLSHandshake )
            m_secure = true;
          else if( op == TLSWrite )
            m_sendBuffer.erase( 0, ret );
          else if( op == TLSRead )
            m_handler->handleDecryptedData( this, std::string( m_buf, ret ) );
          pushFunc();
          break;
        default:
          if( !m_secure )
            m_handler->handleHandshakeResult( this, false, m_certInfo );
          return;
          break;
      }
      if( !onceAgain && !m_recvBuffer.length() )
        onceAgain = true;
      else if( onceAgain )
        onceAgain = false;
    }
    while( ( ( onceAgain || m_recvBuffer.length() ) && ( !m_secure || op == TLSRead ) )
           || ( ( op == TLSWrite ) && ( ret > 0 ) ));
  }

  int OpenSSLBase::openSSLTime2UnixTime( const char* time_string )
  {
    char tstring[19];

    // making seperate c string out of time string
    int m = 0;
    for( int n = 0; n < 12; n += 2 )
    {
      tstring[m] = time_string[n];
      tstring[m + 1] = time_string[n + 1];
      tstring[m + 2] = 0;
      m += 3;
    }

    // converting to struct tm
    tm time_st;
    time_st.tm_year = ( atoi( &tstring[3 * 0] ) >= 70 ) ? atoi( &tstring[3 * 0] )
                                                        : atoi( &tstring[3 * 0] ) + 100;
    time_st.tm_mon = atoi( &tstring[3 * 1] ) - 1;
    time_st.tm_mday = atoi( &tstring[3 * 2] );
    time_st.tm_hour = atoi( &tstring[3 * 3] );
    time_st.tm_min = atoi( &tstring[3 * 4] );
    time_st.tm_sec = atoi( &tstring[3 * 5] );

    time_t unixt = mktime( &time_st );
    return unixt;
  }

  bool OpenSSLBase::handshake()
  {

    doTLSOperation( TLSHandshake );

    if( !m_secure )
      return true;

    int res = SSL_get_verify_result( m_ssl );
    if( res != X509_V_OK )
      m_certInfo.status = CertInvalid;
    else
      m_certInfo.status = CertOk;

    X509* peer = SSL_get_peer_certificate( m_ssl );
    if( peer )
    {
      char peer_CN[256];
      X509_NAME_get_text_by_NID( X509_get_issuer_name( peer ), NID_commonName, peer_CN, sizeof( peer_CN ) );
      m_certInfo.issuer = peer_CN;
      X509_NAME_get_text_by_NID( X509_get_subject_name( peer ), NID_commonName, peer_CN, sizeof( peer_CN ) );
      m_certInfo.server = peer_CN;
      m_certInfo.date_from = openSSLTime2UnixTime( (char*) (peer->cert_info->validity->notBefore->data) );
      m_certInfo.date_to = openSSLTime2UnixTime( (char*) (peer->cert_info->validity->notAfter->data) );
      std::string p( peer_CN );
      std::transform( p.begin(), p.end(), p.begin(), tolower );
      if( p != m_server )
        m_certInfo.status |= CertWrongPeer;

      if( ASN1_UTCTIME_cmp_time_t( X509_get_notBefore( peer ), time( 0 ) ) != -1 )
        m_certInfo.status |= CertNotActive;

      if( ASN1_UTCTIME_cmp_time_t( X509_get_notAfter( peer ), time( 0 ) ) != 1 )
        m_certInfo.status |= CertExpired;
    }
    else
    {
      m_certInfo.status = CertInvalid;
    }

    const char* tmp;
    tmp = SSL_get_cipher_name( m_ssl );
    if( tmp )
      m_certInfo.cipher = tmp;

    tmp = SSL_get_cipher_version( m_ssl );
    if( tmp )
      m_certInfo.protocol = tmp;

    tmp = SSL_COMP_get_name( SSL_get_current_compression( m_ssl ) );
    if( tmp )
      m_certInfo.compression = tmp;

    m_valid = true;

    m_handler->handleHandshakeResult( this, true, m_certInfo );
    return true;
  }

  void OpenSSLBase::pushFunc()
  {
    int wantwrite;
    size_t wantread;
    int frombio;
    int tobio;

    while( ( wantwrite = BIO_ctrl_pending( m_nbio ) ) > 0 )
    {
      if( wantwrite > m_bufsize )
        wantwrite = m_bufsize;

      if( !wantwrite )
        break;

      frombio = BIO_read( m_nbio, m_buf, wantwrite );

      if( m_handler )
        m_handler->handleEncryptedData( this, std::string( m_buf, frombio ) );
    }

    while( ( wantread = BIO_ctrl_get_read_request( m_nbio ) ) > 0 )
    {
      if( wantread > m_recvBuffer.length() )
        wantread = m_recvBuffer.length();

      if( !wantread )
        break;

      tobio = BIO_write( m_nbio, m_recvBuffer.c_str(), wantread );
      m_recvBuffer.erase( 0, tobio );
    }
  }

}

#endif // HAVE_OPENSSL
