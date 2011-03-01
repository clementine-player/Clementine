/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "gpgencrypted.h"
#include "tag.h"

namespace gloox
{

  GPGEncrypted::GPGEncrypted( const std::string& encrypted )
    : StanzaExtension( ExtGPGEncrypted ),
      m_encrypted( encrypted ), m_valid( true )
  {
    if( m_encrypted.empty() )
      m_valid = false;
  }

  GPGEncrypted::GPGEncrypted( const Tag* tag )
    : StanzaExtension( ExtGPGEncrypted ),
      m_valid( false )
  {
    if( tag && tag->name() == "x" && tag->hasAttribute( XMLNS, XMLNS_X_GPGENCRYPTED ) )
    {
      m_valid = true;
      m_encrypted = tag->cdata();
    }
  }

  GPGEncrypted::~GPGEncrypted()
  {
  }

  const std::string& GPGEncrypted::filterString() const
  {
    static const std::string filter = "/message/x[@xmlns='" + XMLNS_X_GPGENCRYPTED + "']";
    return filter;
  }

  Tag* GPGEncrypted::tag() const
  {
    if( !m_valid )
      return 0;

    Tag* x = new Tag( "x", m_encrypted );
    x->addAttribute( XMLNS, XMLNS_X_GPGENCRYPTED );

    return x;
  }

}
