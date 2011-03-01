/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "messagefilter.h"


namespace gloox
{

  MessageFilter::MessageFilter( MessageSession* parent )
    : m_parent( 0 )
  {
    if( parent )
      attachTo( parent );
  }

  MessageFilter::~MessageFilter()
  {
  }

  void MessageFilter::attachTo( MessageSession* session )
  {
    if( m_parent )
      m_parent->removeMessageFilter( this );

    if( session )
      session->registerMessageFilter( this );

    m_parent = session;
  }

}
