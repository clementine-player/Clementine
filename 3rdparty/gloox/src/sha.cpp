/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "config.h"

#include "sha.h"
#include "gloox.h"

#include <cstdio>

namespace gloox
{

  SHA::SHA()
  {
    init();
  }

  SHA::~SHA()
  {
  }

  void SHA::init()
  {
    Length_Low = 0;
    Length_High = 0;
    Message_Block_Index = 0;

    H[0] = 0x67452301;
    H[1] = 0xEFCDAB89;
    H[2] = 0x98BADCFE;
    H[3] = 0x10325476;
    H[4] = 0xC3D2E1F0;

    m_finished = false;
    m_corrupted = false;
  }

  void SHA::reset()
  {
    init();
  }

  const std::string SHA::hex()
  {
    if( m_corrupted )
      return EmptyString;

    finalize();

    char buf[41];
    for( int i = 0; i < 20; ++i )
      sprintf( buf + i * 2, "%02x", (unsigned char)( H[i >> 2] >> ( ( 3 - ( i & 3 ) ) << 3 ) ) );

    return std::string( buf, 40 );
  }

  const std::string SHA::binary()
  {
    if( !m_finished )
      finalize();

    unsigned char digest[20];
    for( int i = 0; i < 20; ++i )
      digest[i] = (unsigned char)( H[i >> 2] >> ( ( 3 - ( i & 3 ) ) << 3 ) );

    return std::string( (char*)digest, 20 );
  }

  void SHA::finalize()
  {
    if( !m_finished )
    {
      pad();
      m_finished = true;
    }
  }

  void SHA::feed( const unsigned char* data, unsigned length )
  {
    if( !length )
      return;

    if( m_finished || m_corrupted )
    {
      m_corrupted = true;
      return;
    }

    while( length-- && !m_corrupted )
    {
      Message_Block[Message_Block_Index++] = ( *data & 0xFF );

      Length_Low += 8;
      Length_Low &= 0xFFFFFFFF;
      if( Length_Low == 0 )
      {
        Length_High++;
        Length_High &= 0xFFFFFFFF;
        if( Length_High == 0 )
        {
          m_corrupted = true;
        }
      }

      if( Message_Block_Index == 64 )
      {
        process();
      }

      ++data;
    }
  }

  void SHA::feed( const std::string& data )
  {
    feed( (const unsigned char*)data.c_str(), (int)data.length() );
  }

  void SHA::process()
  {
    const unsigned K[] = { 0x5A827999,
                           0x6ED9EBA1,
                           0x8F1BBCDC,
                           0xCA62C1D6
                          };
    int t;
    unsigned temp;
    unsigned W[80];
    unsigned A, B, C, D, E;

    for( t = 0; t < 16; t++ )
    {
      W[t] =  ((unsigned) Message_Block[t * 4]) << 24;
      W[t] |= ((unsigned) Message_Block[t * 4 + 1]) << 16;
      W[t] |= ((unsigned) Message_Block[t * 4 + 2]) << 8;
      W[t] |= ((unsigned) Message_Block[t * 4 + 3]);
    }

    for( t = 16; t < 80; ++t )
    {
      W[t] = shift( 1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16] );
    }

    A = H[0];
    B = H[1];
    C = H[2];
    D = H[3];
    E = H[4];

    for( t = 0; t < 20; ++t )
    {
      temp = shift( 5, A ) + ( ( B & C ) | ( ( ~B ) & D ) ) + E + W[t] + K[0];
      temp &= 0xFFFFFFFF;
      E = D;
      D = C;
      C = shift( 30, B );
      B = A;
      A = temp;
    }

    for( t = 20; t < 40; ++t )
    {
      temp = shift( 5, A ) + ( B ^ C ^ D ) + E + W[t] + K[1];
      temp &= 0xFFFFFFFF;
      E = D;
      D = C;
      C = shift( 30, B );
      B = A;
      A = temp;
    }

    for( t = 40; t < 60; ++t )
    {
      temp = shift( 5, A ) + ( ( B & C ) | ( B & D ) | ( C & D ) ) + E + W[t] + K[2];
      temp &= 0xFFFFFFFF;
      E = D;
      D = C;
      C = shift( 30, B );
      B = A;
      A = temp;
    }

    for( t = 60; t < 80; ++t )
    {
      temp = shift( 5, A ) + ( B ^ C ^ D ) + E + W[t] + K[3];
      temp &= 0xFFFFFFFF;
      E = D;
      D = C;
      C = shift( 30, B );
      B = A;
      A = temp;
    }

    H[0] = ( H[0] + A ) & 0xFFFFFFFF;
    H[1] = ( H[1] + B ) & 0xFFFFFFFF;
    H[2] = ( H[2] + C ) & 0xFFFFFFFF;
    H[3] = ( H[3] + D ) & 0xFFFFFFFF;
    H[4] = ( H[4] + E ) & 0xFFFFFFFF;

    Message_Block_Index = 0;
  }

  void SHA::pad()
  {
    Message_Block[Message_Block_Index++] = 0x80;

    if( Message_Block_Index > 55 )
    {
      while( Message_Block_Index < 64 )
      {
        Message_Block[Message_Block_Index++] = 0;
      }

      process();
    }

    while( Message_Block_Index < 56 )
    {
      Message_Block[Message_Block_Index++] = 0;
    }

    Message_Block[56] = static_cast<unsigned char>( ( Length_High >> 24 ) & 0xFF );
    Message_Block[57] = static_cast<unsigned char>( ( Length_High >> 16 ) & 0xFF );
    Message_Block[58] = static_cast<unsigned char>( ( Length_High >> 8 ) & 0xFF );
    Message_Block[59] = static_cast<unsigned char>( ( Length_High ) & 0xFF );
    Message_Block[60] = static_cast<unsigned char>( ( Length_Low >> 24 ) & 0xFF );
    Message_Block[61] = static_cast<unsigned char>( ( Length_Low >> 16 ) & 0xFF );
    Message_Block[62] = static_cast<unsigned char>( ( Length_Low >> 8 ) & 0xFF );
    Message_Block[63] = static_cast<unsigned char>( ( Length_Low ) & 0xFF );

    process();
  }


  unsigned SHA::shift( int bits, unsigned word )
  {
    return ( ( word << bits ) & 0xFFFFFFFF) | ( ( word & 0xFFFFFFFF ) >> ( 32-bits ) );
  }

}
