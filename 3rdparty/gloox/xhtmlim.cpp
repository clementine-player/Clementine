/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "xhtmlim.h"

#include "tag.h"

namespace gloox
{

  XHtmlIM::XHtmlIM( const Tag* xhtml )
    : StanzaExtension( ExtXHtmlIM ), m_xhtml( 0 )
  {
    if( !xhtml|| xhtml->name() != "html" || xhtml->xmlns() != XMLNS_XHTML_IM )
      return;

    if( !xhtml->hasChild( "body", XMLNS, "http://www.w3.org/1999/xhtml" ) )
      return;

    m_xhtml = xhtml->clone();
  }

  XHtmlIM::~XHtmlIM()
  {
    delete m_xhtml;
  }

  const std::string& XHtmlIM::filterString() const
  {
    static const std::string filter = "/message/html[@xmlns='" + XMLNS_XHTML_IM + "']";
    return filter;
  }

  Tag* XHtmlIM::tag() const
  {
    return m_xhtml->clone();
  }

  StanzaExtension* XHtmlIM::clone() const
  {
    XHtmlIM* x = new XHtmlIM();
    x->m_xhtml = m_xhtml ? m_xhtml->clone() : 0;
    return 0;
  }

}
