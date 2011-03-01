/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "util.h"
#include "gloox.h"

namespace gloox
{

  namespace util
  {

    int internalLog2( unsigned int n )
    {
      int pos = 0;
      if ( n >= 1<<16 ) { n >>= 16; pos += 16; }
      if ( n >= 1<< 8 ) { n >>=  8; pos +=  8; }
      if ( n >= 1<< 4 ) { n >>=  4; pos +=  4; }
      if ( n >= 1<< 2 ) { n >>=  2; pos +=  2; }
      if ( n >= 1<< 1 ) {           pos +=  1; }
      return ( (n == 0) ? (-1) : pos );
    }

    unsigned _lookup( const std::string& str, const char* values[], unsigned size, int def )
    {
      unsigned i = 0;
      for( ; i < size && str != values[i]; ++i )
        ;
      return ( i == size && def >= 0 ) ? (unsigned)def : i;
    }

    const std::string _lookup( unsigned code, const char* values[], unsigned size, const std::string& def )
    {
      return code < size ? std::string( values[code] ) : def;
    }

    unsigned _lookup2( const std::string& str, const char* values[],
                       unsigned size, int def )
    {
      return 1 << _lookup( str, values, size, def <= 0 ? def : (int)internalLog2( def ) );
    }

    const std::string _lookup2( unsigned code, const char* values[], unsigned size, const std::string& def )
    {
      const unsigned i = (unsigned)internalLog2( code );
      return i < size ? std::string( values[i] ) : def;
    }

    static const char escape_chars[] = { '&', '<', '>', '\'', '"' };

    static const std::string escape_seqs[] = { "amp;", "lt;", "gt;", "apos;", "quot;" };

    static const unsigned escape_size = 5;

    const std::string escape( std::string what )
    {
      for( size_t val, i = 0; i < what.length(); ++i )
      {
        for( val = 0; val < escape_size; ++val )
        {
          if( what[i] == escape_chars[val] )
          {
            what[i] = '&';
            what.insert( i+1, escape_seqs[val] );
            i += escape_seqs[val].length();
            break;
          }
        }
      }
      return what;
    }

    bool checkValidXMLChars( const std::string& data )
    {
      if( data.empty() )
        return true;

      std::string::const_iterator it = data.begin();
      for( ; it != data.end()
             && ( (unsigned char)(*it) == 0x09
                  || (unsigned char)(*it) == 0x0a
                  || (unsigned char)(*it) == 0x0d
                  || ( (unsigned char)(*it) >= 0x20
                     && (unsigned char)(*it) != 0xc0
                     && (unsigned char)(*it) != 0xc1
                     && (unsigned char)(*it) < 0xf5 ) ); ++it )
        ;

      return ( it == data.end() );
    }

    void replaceAll( std::string& target, const std::string& find, const std::string& replace )
    {
      std::string::size_type findSize = find.size();
      std::string::size_type replaceSize = replace.size();

      if( findSize == 0 )
        return;

      std::string::size_type index = target.find( find, 0 );

      while( index != std::string::npos )
      {
        target.replace( index, findSize, replace );
        index = target.find( find, index+replaceSize );
      }
    }

  }

}

