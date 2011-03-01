/*
 * Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
 * This file is part of the gloox library. http://camaya.net/gloox
 *
 * This software is distributed under a license. The full license
 * agreement can be found in the file LICENSE in this distribution.
 * This software may not be copied, modified, sold or distributed
 * other than expressed in the named license agreement.
 *
 * This software is distributed without any warranty.
 */

#include "compressiondefault.h"

#include "compressiondatahandler.h"

#include "config.h"

#if defined( HAVE_ZLIB )
# define HAVE_COMPRESSION
# include "compressionzlib.h"
#endif

// #if defined( HAVE_LZW )
// # define HAVE_COMPRESSION
// # include "compressionlzw.h"
// #endif

namespace gloox
{

  CompressionDefault::CompressionDefault( CompressionDataHandler* cdh, Method method )
    : CompressionBase( cdh ), m_impl( 0 )
  {
    switch( method )
    {
      case MethodZlib:
#ifdef HAVE_ZLIB
        m_impl = new CompressionZlib( cdh );
#endif
        break;
      case MethodLZW:
#ifdef HAVE_LZW
        m_impl = new CompressionLZW( cdh );
#endif
        break;
      default:
        break;
    }
  }

  CompressionDefault::~CompressionDefault()
  {
    delete m_impl;
  }

  bool CompressionDefault::init()
  {
    return m_impl ? m_impl->init() : false;
  }

  int CompressionDefault::types()
  {
    int types = 0;
#ifdef HAVE_ZLIB
    types |= MethodZlib;
#endif
#ifdef HAVE_LZW
    types |= MethodLZW;
#endif
    return types;
  }

  void CompressionDefault::compress( const std::string& data )
  {
    if( m_impl )
      m_impl->compress( data );
  }

  void CompressionDefault::decompress( const std::string& data )
  {
    if( m_impl )
      m_impl->decompress( data );
  }

  void CompressionDefault::cleanup()
  {
    if( m_impl )
      m_impl->cleanup();
  }

}
