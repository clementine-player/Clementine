/*
  Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "attention.h"
#include "tag.h"

namespace gloox
{

  Attention::Attention()
    : StanzaExtension( ExtAttention )
  {
  }

  Attention::~Attention()
  {
  }

  const std::string& Attention::filterString() const
  {
    static const std::string filter = "/message/attention[@xmlns='" + XMLNS_ATTENTION + "']";
    return filter;
  }

  Tag* Attention::tag() const
  {
    Tag* t = new Tag( "attention" );
    t->setXmlns( XMLNS_ATTENTION );
    return t;
  }

}
