/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef PARSER_H__
#define PARSER_H__

#include "gloox.h"
#include "taghandler.h"
#include "tag.h"

#include <string>

namespace gloox
{


  /**
   * @brief This class implements an XML parser.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API Parser
  {
    public:
      /**
       * Constructs a new Parser object.
       * @param ph The object to send incoming Tags to.
       * @param deleteRoot Indicates whether a parsed Tag should be
       * deleted after pushing it upstream. Defaults to @p true.
       */
      Parser( TagHandler* ph, bool deleteRoot = true );

      /**
       * Virtual destructor.
       */
      virtual ~Parser();

      /**
       * Use this function to feed the parser with more XML.
       * @param data Raw xml to parse. It may be modified if backbuffering is necessary.
       * @return Returns @b -1 if parsing was successful. If a parse error occured, the
       * character position where the error was occured is returned.
       */
      int feed( std::string& data );

      /**
       * Resets internal state.
       * @param deleteRoot Whether to delete the m_root member. For
       * internal use only.
       */
      void cleanup( bool deleteRoot = true );

    private:
      enum ParserInternalState
      {
        Initial,
        InterTag,
        TagOpening,
        TagOpeningSlash,
        TagOpeningLt,
        TagInside,
        TagNameCollect,
        TagNameComplete,
        TagNameAlmostComplete,
        TagAttribute,
        TagAttributeComplete,
        TagAttributeEqual,
        TagClosing,
        TagClosingSlash,
        TagValueApos,
        TagAttributeValue,
        TagPreamble,
        TagCDATASection
      };

      enum ForwardScanState
      {
        ForwardFound,
        ForwardNotFound,
        ForwardInsufficientSize
      };

      enum DecodeState
      {
        DecodeValid,
        DecodeInvalid,
        DecodeInsufficient
      };

      void addTag();
      void addAttribute();
      void addCData();
      bool closeTag();
      bool isWhitespace( unsigned char c );
      bool isValid( unsigned char c );
      void streamEvent( Tag* tag );
      ForwardScanState forwardScan( std::string::size_type& pos, const std::string& data,
                                    const std::string& needle );
      DecodeState decode( std::string::size_type& pos, const std::string& data );

      TagHandler* m_tagHandler;
      Tag* m_current;
      Tag* m_root;
      StringMap* m_xmlnss;

      ParserInternalState m_state;
      Tag::AttributeList m_attribs;
      std::string m_tag;
      std::string m_cdata;
      std::string m_attrib;
      std::string m_value;
      std::string m_xmlns;
      std::string m_tagPrefix;
      std::string m_attribPrefix;
      std::string m_backBuffer;
      int m_preamble;
      bool m_quote;
      bool m_haveTagPrefix;
      bool m_haveAttribPrefix;
      bool m_attribIsXmlns;
      bool m_deleteRoot;

  };

}

#endif // PARSER_H__
