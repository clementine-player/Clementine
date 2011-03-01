/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "privacyitem.h"

namespace gloox
{

  PrivacyItem::PrivacyItem( const ItemType type, const ItemAction action,
                            const int packetType, const std::string& value )
    : m_type( type ), m_action( action ), m_packetType( packetType ),
      m_value( value )
  {
  }

  PrivacyItem::~PrivacyItem()
  {
  }

  bool PrivacyItem::operator==( const PrivacyItem& item ) const
  {
    if( m_type == item.type()
        && m_action == item.action()
        && m_packetType == item.packetType()
        && m_value == item.value() )
      return true;
    else
      return false;
  }

}
