/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "gloox.h"
#include "util.h"
#include "parser.h"

#include <cstdlib>

namespace gloox
{

  Parser::Parser( TagHandler* ph, bool deleteRoot )
    : m_tagHandler( ph ), m_current( 0 ), m_root( 0 ), m_xmlnss( 0 ), m_state( Initial ),
      m_preamble( 0 ), m_quote( false ), m_haveTagPrefix( false ), m_haveAttribPrefix( false ),
      m_attribIsXmlns( false ), m_deleteRoot( deleteRoot )
  {
  }

  Parser::~Parser()
  {
    delete m_root;
    delete m_xmlnss;
  }

  Parser::DecodeState Parser::decode( std::string::size_type& pos, const std::string& data )
  {
    std::string::size_type p = data.find( ';', pos );
    std::string::size_type diff = p - pos;

    if( p == std::string::npos )
    {
      m_backBuffer = data.substr( pos );
      return DecodeInsufficient;
    }

    if( diff < 3 || diff > 9 )
      return DecodeInvalid;

    std::string rep;
    switch( data[pos + 1] )
    {
      case '#':
        {
          int base = 10;
          int idx = 2;

          if( data[pos + 2] == 'x' || data[pos + 2] == 'X' )
          {
            base = 16;
            idx = 3;
          }

          char* end;
          const long int val = std::strtol( data.data() + pos + idx, &end, base );
          if( *end != ';' || val < 0 )
            return DecodeInvalid;

          if( val == 0x9 || val == 0xA || val == 0xD || ( val >= 0x20 && val <= 0x7F ) )
          {
            rep += char( val );
          }
          else if( val >= 0x80 && val <= 0x7FF )
          {
            rep += char( 192 + ( val >> 6 ) );
            rep += char( 128 + ( val % 64 ) );
          }
          else if( ( val >= 0x800 && val <= 0xD7FF ) || ( val >= 0xE000 && val <= 0xFFFD ) )
          {
            rep += char( 224 + ( val >> 12 ) );
            rep += char( 128 + ( ( val >> 6 ) % 64 ) );
            rep += char( 128 + ( val % 64 ) );
          }
          else if( val >= 0x100000 && val < 0x10FFFF )
          {
            rep += char( 240 + ( val >> 18 ) );
            rep += char( 128 + ( ( val >> 12 ) % 64 ) );
            rep += char( 128 + ( ( val >> 6 ) % 64 ) );
            rep += char( 128 + ( val % 64 ) );
          }
          else
            return DecodeInvalid;
        }
        break;
      case 'l':
        if( diff == 3 && data[pos + 2] == 't' )
          rep += '<';
        else
          return DecodeInvalid;
        break;
      case 'g':
        if( diff == 3 && data[pos + 2] == 't' )
          rep += '>';
        else
          return DecodeInvalid;
        break;
      case 'a':
        if( diff == 5 && !data.compare( pos + 1, 5, "apos;" ) )
          rep += '\'';
        else if( diff == 4 && !data.compare( pos + 1, 4, "amp;" ) )
          rep += '&';
        else
          return DecodeInvalid;
        break;
      case 'q':
        if( diff == 5 && !data.compare( pos + 1, 5, "quot;" ) )
          rep += '"';
        else
          return DecodeInvalid;
        break;
      default:
        return DecodeInvalid;
    }

    switch( m_state )
    {
      case TagInside:
        m_cdata += rep;
        break;
      case TagAttributeValue:
        m_value += rep;
        break;
      default:
        break;
    }
    pos += diff;
    return DecodeValid;
  }

  Parser::ForwardScanState Parser::forwardScan( std::string::size_type& pos, const std::string& data,
                                                const std::string& needle )
  {
    if( pos + needle.length() <= data.length() )
    {
      if( !data.compare( pos, needle.length(), needle ) )
      {
        pos += needle.length() - 1;
        return ForwardFound;
      }
      else
      {
        return ForwardNotFound;
      }
    }
    else
    {
      m_backBuffer = data.substr( pos );
      return ForwardInsufficientSize;
    }
  }

  int Parser::feed( std::string& data )
  {
    if( !m_backBuffer.empty() )
    {
      data.insert( 0, m_backBuffer );
      m_backBuffer = EmptyString;
    }

    std::string::size_type count = data.length();
    for( std::string::size_type i = 0; i < count; ++i )
    {
      const unsigned char c = data[i];
//       printf( "found char:   %c, ", c );

      if( !isValid( c ) )
      {
        cleanup();
        return static_cast<int>( i );
      }

      switch( m_state )
      {
        case Initial:
//           printf( "Initial: %c\n", c );
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '<':
              m_state = TagOpening;
              break;
            default:
              cleanup();
              return static_cast<int>( i );
              break;
          }
          break;
        case InterTag:
//           printf( "InterTag: %c\n", c );
          m_tag = EmptyString;
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '<':
              m_state = TagOpening;
              break;
            case '>':
            default:
              if( m_current )
              {
                m_cdata += c;
                m_state = TagInside;
              }
              break;
          }
          break;
          case TagOpening:               // opening '<' has been found before
//           printf( "TagOpening: %c\n", c );
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '<':
            case '>':
            case '&':
              cleanup();
              return static_cast<int>( i );
              break;
            case '/':
              m_state = TagClosingSlash;
              break;
            case '?':
              m_state = TagNameCollect;
              m_preamble = 1;
              break;
            case '!':
              switch( forwardScan( i, data, "![CDATA[" ) )
              {
                case ForwardFound:
                  m_state = TagCDATASection;
                  break;
                case ForwardNotFound:
                  cleanup();
                  return static_cast<int>( i );
                case ForwardInsufficientSize:
                  return -1;
              }
              break;
            default:
              m_tag += c;
              m_state = TagNameCollect;
              break;
          }
          break;
        case TagCDATASection:
          switch( c )
          {
            case ']':
              switch( forwardScan( i, data, "]]>" ) )
              {
                case ForwardFound:
                  m_state = TagInside;
                  break;
                case ForwardNotFound:
                  m_cdata += c;
                  break;
                case ForwardInsufficientSize:
                  return -1;
              }
              break;
            default:
              m_cdata += c;
              break;
          }
          break;
        case TagNameCollect:          // we're collecting the tag's name, we have at least one octet already
//           printf( "TagNameCollect: %c\n", c );
          if( isWhitespace( c ) )
          {
            m_state = TagNameComplete;
            break;
          }

          switch( c )
          {
            case '<':
            case '?':
            case '!':
            case '&':
              cleanup();
              return static_cast<int>( i );
              break;
            case '/':
              m_state = TagOpeningSlash;
              break;
            case '>':
              addTag();
              m_state = TagInside;
              break;
            case ':':
              if( !m_haveTagPrefix )
              {
                m_haveTagPrefix = true;
                m_tagPrefix = m_tag;
                m_tag = EmptyString;
              }
              else
              {
                cleanup();
                return static_cast<int>( i );
              }
              break;
            default:
              m_tag += c;
              break;
          }
          break;
        case TagInside:                // we're inside a tag, expecting a child tag or cdata
//           printf( "TagInside: %c\n", c );
          m_tag = EmptyString;
          switch( c )
          {
            case '<':
              addCData();
              m_state = TagOpening;
              break;
            case '&':
//               printf( "TagInside, calling decode\n" );
              switch( decode( i, data ) )
              {
                case DecodeValid:
                  break;
                case DecodeInvalid:
                  cleanup();
                  return static_cast<int>( i );
                case DecodeInsufficient:
                  return -1;
              }
              break;
            default:
              m_cdata += c;
              break;
          }
          break;
        case TagOpeningSlash:         // a slash in an opening tag has been found, initing close of the tag
//           printf( "TagOpeningSlash: %c\n", c );
          if( isWhitespace( c ) )
            break;

          if( c == '>' )
          {
            addTag();
            if( !closeTag() )
            {
//               printf( "noipe, here\n" );
              cleanup();
              return static_cast<int>( i );
            }

            m_state = InterTag;
          }
          else
          {
            cleanup();
            return static_cast<int>( i );
          }
          break;
        case TagClosingSlash:         // we have found the '/' of a closing tag
//           printf( "TagClosingSlash: %c\n", c );
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '>':
            case '<':
            case '/':
              cleanup();
              return static_cast<int>( i );
              break;
            default:
              m_tag += c;
              m_state = TagClosing;
              break;
          }
          break;
        case TagClosing:               // we're collecting the name of a closing tag
//           printf( "TagClosing: %c\n", c );
          switch( c )
          {
            case '<':
            case '/':
            case '!':
            case '?':
            case '&':
              cleanup();
              return static_cast<int>( i );
              break;
            case ':':
              if( !m_haveTagPrefix )
              {
                m_haveTagPrefix = true;
                m_tagPrefix = m_tag;
                m_tag = EmptyString;
              }
              else
              {
                cleanup();
                return static_cast<int>( i );
              }
              break;
            case '>':
              if( !closeTag() )
              {
//                 printf( "here\n" );
                cleanup();
                return static_cast<int>( i );
              }
              m_state = InterTag;
              break;
            default:
              m_tag += c;
              break;
          }
          break;
        case TagNameComplete:        // a tag name is complete, expect tag close or attribs
//           printf( "TagNameComplete: %c\n", c );
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '<':
            case '!':
            case '&':
              cleanup();
              return static_cast<int>( i );
              break;
            case '/':
              m_state = TagOpeningSlash;
              break;
            case '>':
              if( m_preamble == 1 )
              {
                cleanup();
                return static_cast<int>( i );
              }
              m_state = TagInside;
              addTag();
              break;
            case '?':
              if( m_preamble == 1 )
                m_preamble = 2;
              else
              {
                cleanup();
                return static_cast<int>( i );
              }
              break;
            default:
              m_attrib += c;
              m_state = TagAttribute;
              break;
          }
          break;
        case TagAttribute:                  // we're collecting the name of an attribute, we have at least 1 octet
//           printf( "TagAttribute: %c\n", c );
          if( isWhitespace( c ) )
          {
            m_state = TagAttributeComplete;
            break;
          }

          switch( c )
          {
            case '<':
            case '/':
            case '>':
            case '?':
            case '!':
            case '&':
              cleanup();
              return static_cast<int>( i );
              break;
            case '=':
              m_state = TagAttributeEqual;
              break;
            case ':':
              if( !m_haveAttribPrefix && m_attrib != XMLNS )
              {
                m_haveAttribPrefix = true;
                m_attribPrefix = m_attrib;
                m_attrib = EmptyString;
              }
              else if( m_attrib == XMLNS )
              {
                m_attribIsXmlns = true;
                m_attrib = EmptyString;
              }
              else
              {
                cleanup();
                return static_cast<int>( i );
              }
              break;
            default:
              m_attrib += c;
          }
          break;
        case TagAttributeComplete:         // we're expecting an equals sign or ws
//           printf( "TagAttributeComplete: %c\n", c );
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '=':
              m_state = TagAttributeEqual;
              break;
            default:
              cleanup();
              return static_cast<int>( i );
              break;
          }
          break;
        case TagAttributeEqual:            // we have found an equals sign
//           printf( "TagAttributeEqual: %c\n", c );
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '"':
              m_quote = true;
            case '\'':
              m_state = TagAttributeValue;
              break;
            default:
              cleanup();
              return static_cast<int>( i );
              break;
          }
          break;
        case TagAttributeValue:                 // we're expecting value data
//           printf( "TagValue: %c\n", c );
          switch( c )
          {
            case '<':
              cleanup();
              return static_cast<int>( i );
              break;
            case '\'':
              if( m_quote )
              {
                m_value += c;
                break;
              }
            case '"':
              addAttribute();
              m_state = TagNameAlmostComplete;
              m_quote = false;
              break;
            case '&':
//               printf( "TagAttributeValue, calling decode\n" );
              switch( decode( i, data ) )
              {
                case DecodeValid:
                  break;
                case DecodeInvalid:
                  cleanup();
                  return static_cast<int>( i );
                case DecodeInsufficient:
                  return -1;
              }
              break;
            case '>':
            default:
              m_value += c;
          }
          break;
        case TagNameAlmostComplete:
//           printf( "TagAttributeEqual: %c\n", c );
          if( isWhitespace( c ) )
          {
            m_state = TagNameComplete;
            break;
          }

          switch( c )
          {
            case '/':
              m_state = TagOpeningSlash;
              break;
            case '>':
              if( m_preamble == 1 )
              {
                cleanup();
                return static_cast<int>( i );
              }
              m_state = TagInside;
              addTag();
              break;
            case '?':
              if( m_preamble == 1 )
                m_preamble = 2;
              else
              {
                cleanup();
                return static_cast<int>( i );
              }
              break;
            default:
              cleanup();
              return static_cast<int>( i );
              break;
          }
          break;
        default:
//           printf( "default action!?\n" );
          break;
      }
//       printf( "parser state: %d\n", m_state );
    }

    return -1;
  }

  void Parser::addTag()
  {
    if( !m_root )
    {
//       printf( "created Tag named %s, ", m_tag.c_str() );
      m_root = new Tag( m_tag );
      m_current = m_root;
    }
    else
    {
//       printf( "created Tag named %s, ", m_tag.c_str() );
      m_current = new Tag( m_current, m_tag );
    }

    if( m_haveTagPrefix )
    {
//       printf( "setting tag prefix: %s\n", m_tagPrefix.c_str() );
      m_current->setPrefix( m_tagPrefix );
      m_haveTagPrefix = false;
    }

    if( m_attribs.size() )
    {
      m_current->setAttributes( m_attribs );
//       printf( "added %d attributes, ", m_attribs.size() );
      m_attribs.clear();
    }

    if( m_xmlnss )
    {
//       printf( "have ns decls\n" );
//       StringMap::const_iterator it = m_xmlnss->begin();
//       for( ; it != m_xmlnss->end(); ++it )
//         printf( "%s='%s'\n", (*it).first.c_str(), (*it).second.c_str() );
      m_current->setXmlns( m_xmlnss );
      m_xmlnss = 0;
    }

    m_current->setXmlns( m_xmlns );
    m_xmlns = EmptyString;

    if( m_tag == "stream" && m_root->xmlns() == XMLNS_STREAM )
    {
      streamEvent( m_root );
      cleanup( m_deleteRoot );
      return;
    }
//     else
//       printf( "%s, ", m_root->xml().c_str() );

    if( m_root && m_root == m_current && m_tagPrefix == "stream" )
      m_root->setXmlns( XMLNS_STREAM, m_tagPrefix );

    if( m_tag == "xml" && m_preamble == 2 )
      cleanup();
  }

  void Parser::addAttribute()
  {
    Tag::Attribute* attr = new Tag::Attribute( m_attrib, m_value );;
    if( m_attribIsXmlns )
    {
      if( !m_xmlnss )
        m_xmlnss = new StringMap();

      (*m_xmlnss)[m_attrib] = m_value;
      attr->setPrefix( XMLNS );
    }
    else
    {
//   printf( "adding attribute: %s:%s='%s'\n", m_attribPrefix.c_str(), m_attrib.c_str(), m_value.c_str() );
      if( !m_attribPrefix.empty() )
        attr->setPrefix( m_attribPrefix );
      if( m_attrib == XMLNS )
        m_xmlns = m_value;
    }
    m_attribs.push_back( attr );
    m_attrib = EmptyString;
    m_value = EmptyString;
    m_attribPrefix = EmptyString;
    m_haveAttribPrefix = false;
    m_attribIsXmlns = false;
  }

  void Parser::addCData()
  {
    if( m_current && !m_cdata.empty() )
    {
      m_current->addCData( m_cdata );
//       printf( "added cdata %s to %s: %s\n",
//               m_cdata.c_str(), m_current->name().c_str(), m_current->xml().c_str() );
      m_cdata = EmptyString;
    }
  }

  bool Parser::closeTag()
  {
//     printf( "about to close, " );

    if( m_tag == "stream" && m_tagPrefix == "stream" )
      return true;

    if( !m_current || m_current->name() != m_tag
        || ( !m_current->prefix().empty() && m_current->prefix() != m_tagPrefix ) )
    {
//       printf( "current xml: %s\n", m_current->xml().c_str() );
//       printf( "current name: %s, m_tag: %s\n", m_current->name().c_str(), m_tag.c_str() );
//       printf( "current prefix: %s, m_tagPrefix: %s\n", m_current->prefix().c_str(), m_tagPrefix.c_str() );
      return false;
    }

//       printf( "m_current: %s, ", m_current->name().c_str() );
//       printf( "m_tag: %s, ", m_tag.c_str() );

    m_tagPrefix = EmptyString;
    m_haveTagPrefix = false;

    if( m_current->parent() )
      m_current = m_current->parent();
    else
    {
//       printf( "pushing upstream\n" );
      streamEvent( m_root );
      cleanup( m_deleteRoot );
    }

    return true;
  }

  void Parser::cleanup( bool deleteRoot )
  {
    if( deleteRoot )
      delete m_root;
    m_root = 0;
    m_current = 0;
    delete m_xmlnss;
    m_xmlnss = 0;
    m_cdata = EmptyString;
    m_tag = EmptyString;
    m_attrib = EmptyString;
    m_attribPrefix = EmptyString;
    m_tagPrefix = EmptyString;
    m_haveAttribPrefix = false;
    m_haveTagPrefix = false;
    m_value = EmptyString;
    m_xmlns = EmptyString;
    util::clearList( m_attribs );
    m_attribs.clear();
    m_state = Initial;
    m_preamble = 0;
  }

  bool Parser::isValid( unsigned char c )
  {
    return ( c != 0xc0 || c != 0xc1 || c < 0xf5 );
  }

  bool Parser::isWhitespace( unsigned char c )
  {
    return ( c == 0x09 || c == 0x0a || c == 0x0d || c == 0x20 );
  }

  void Parser::streamEvent( Tag* tag )
  {
    if( m_tagHandler )
      m_tagHandler->handleTag( tag );
  }

}
