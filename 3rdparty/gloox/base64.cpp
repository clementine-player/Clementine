/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "base64.h"

namespace gloox
{

  namespace Base64
  {

    static const std::string alphabet64( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );
    static const char pad = '=';
    static const char np  = (char)std::string::npos;
    static char table64vals[] =
    {
      62, np, np, np, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, np, np, np, np, np,
      np, np,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
      18, 19, 20, 21, 22, 23, 24, 25, np, np, np, np, np, np, 26, 27, 28, 29, 30, 31,
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    inline char table64( unsigned char c )
    {
      return ( c < 43 || c > 122 ) ? np : table64vals[c-43];
    }

    const std::string encode64( const std::string& input )
    {
      std::string encoded;
      char c;
      const std::string::size_type length = input.length();

      encoded.reserve( length * 2 );

      for( std::string::size_type i = 0; i < length; ++i )
      {
        c = static_cast<char>( ( input[i] >> 2 ) & 0x3f );
        encoded += alphabet64[c];

        c = static_cast<char>( ( input[i] << 4 ) & 0x3f );
        if( ++i < length )
          c = static_cast<char>( c | static_cast<char>( ( input[i] >> 4 ) & 0x0f ) );
        encoded += alphabet64[c];

        if( i < length )
        {
          c = static_cast<char>( ( input[i] << 2 ) & 0x3c );
          if( ++i < length )
            c = static_cast<char>( c | static_cast<char>( ( input[i] >> 6 ) & 0x03 ) );
          encoded += alphabet64[c];
        }
        else
        {
          ++i;
          encoded += pad;
        }

        if( i < length )
        {
          c = static_cast<char>( input[i] & 0x3f );
          encoded += alphabet64[c];
        }
        else
        {
          encoded += pad;
        }
      }

      return encoded;
    }

    const std::string decode64( const std::string& input )
    {
      char c, d;
      const std::string::size_type length = input.length();
      std::string decoded;

      decoded.reserve( length );

      for( std::string::size_type i = 0; i < length; ++i )
      {
        c = table64(input[i]);
        ++i;
        d = table64(input[i]);
        c = static_cast<char>( ( c << 2 ) | ( ( d >> 4 ) & 0x3 ) );
        decoded += c;
        if( ++i < length )
        {
          c = input[i];
          if( pad == c )
            break;

          c = table64(input[i]);
          d = static_cast<char>( ( ( d << 4 ) & 0xf0 ) | ( ( c >> 2 ) & 0xf ) );
          decoded += d;
        }

        if( ++i < length )
        {
          d = input[i];
          if( pad == d )
            break;

          d = table64(input[i]);
          c = static_cast<char>( ( ( c << 6 ) & 0xc0 ) | d );
          decoded += c;
        }
      }

      return decoded;
    }

  }

}
