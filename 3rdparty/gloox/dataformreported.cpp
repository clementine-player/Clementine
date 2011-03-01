 /*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "dataformreported.h"

#include "tag.h"

namespace gloox
{

  DataFormReported::DataFormReported()
  {
  }

  DataFormReported::DataFormReported( Tag* tag )
  {
    if( tag->name() != "reported" )
      return;

    const TagList &l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      DataFormField* f = new DataFormField( (*it) );
      m_fields.push_back( f );
    }
  }

  DataFormReported::~DataFormReported()
  {
  }

  Tag* DataFormReported::tag() const
  {
    Tag* r = new Tag ( "reported" );
    DataFormFieldContainer::FieldList::const_iterator it = m_fields.begin();
    for( ; it != m_fields.end(); ++it )
    {
      r->addChild( (*it)->tag() );
    }
    return r;
  }

}
