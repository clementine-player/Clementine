/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "compressionzlib.h"

#ifdef HAVE_ZLIB

namespace gloox
{

  CompressionZlib::CompressionZlib( CompressionDataHandler* cdh )
    : CompressionBase( cdh )
  {
  }

  bool CompressionZlib::init()
  {
    int ret = Z_OK;
    m_zinflate.zalloc = Z_NULL;
    m_zinflate.zfree = Z_NULL;
    m_zinflate.opaque = Z_NULL;
    m_zinflate.avail_in = 0;
    m_zinflate.next_in = Z_NULL;
    ret = inflateInit( &m_zinflate );
    if( ret != Z_OK )
      return false;

    m_zdeflate.zalloc = Z_NULL;
    m_zdeflate.zfree = Z_NULL;
    m_zdeflate.opaque = Z_NULL;
    m_zinflate.avail_in = 0;
    m_zinflate.next_in = Z_NULL;
    ret = deflateInit( &m_zdeflate, Z_BEST_COMPRESSION/*Z_DEFAULT_COMPRESSION*/ );
    if( ret != Z_OK )
      return false;

    m_valid = true;
    return true;
  }

  CompressionZlib::~CompressionZlib()
  {
    cleanup();
  }

  void CompressionZlib::compress( const std::string& data )
  {
    if( !m_valid )
      init();

    if( !m_valid || !m_handler || data.empty() )
      return;

    long unsigned int CHUNK = data.length() + ( data.length() / 100 ) + 13;
    Bytef* out = new Bytef[CHUNK];
    char* in = const_cast<char*>( data.c_str() );

    m_compressMutex.lock();

    m_zdeflate.avail_in = static_cast<uInt>( data.length() );
    m_zdeflate.next_in = (Bytef*)in;

    int ret;
    std::string result;
    do {
      m_zdeflate.avail_out = static_cast<uInt>( CHUNK );
      m_zdeflate.next_out = (Bytef*)out;

      ret = deflate( &m_zdeflate, Z_SYNC_FLUSH );
      result.append( (char*)out, CHUNK - m_zdeflate.avail_out );
    } while( m_zdeflate.avail_out == 0 );

    m_compressMutex.unlock();

    delete[] out;

    m_handler->handleCompressedData( result );
  }

  void CompressionZlib::decompress( const std::string& data )
  {
    if( !m_valid )
      init();

    if( !m_valid || !m_handler || data.empty() )
      return;

    int CHUNK = 50;
    char* out = new char[CHUNK];
    char* in = const_cast<char*>( data.c_str() );

    m_zinflate.avail_in = static_cast<uInt>( data.length() );
    m_zinflate.next_in = (Bytef*)in;

    int ret = Z_OK;
    std::string result;
    do
    {
      m_zinflate.avail_out = CHUNK;
      m_zinflate.next_out = (Bytef*)out;

      ret = inflate( &m_zinflate, Z_SYNC_FLUSH );
      result.append( out, CHUNK - m_zinflate.avail_out );
    } while( m_zinflate.avail_out == 0 );

    delete[] out;

    m_handler->handleDecompressedData( result );
  }

  void CompressionZlib::cleanup()
  {
    if( !m_valid )
      return;

    inflateEnd( &m_zinflate );
    deflateEnd( &m_zdeflate );

    m_valid = false;
  }

}

#endif // HAVE_ZLIB
