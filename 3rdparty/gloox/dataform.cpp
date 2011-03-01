/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "dataform.h"
#include "dataformfield.h"
#include "dataformitem.h"
#include "dataformreported.h"
#include "util.h"
#include "tag.h"

namespace gloox
{

  DataForm::DataForm( FormType type, const StringList& instructions, const std::string& title )
    : StanzaExtension( ExtDataForm ),
      m_type( type ), m_instructions( instructions ), m_title( title ), m_reported( 0 )
  {
  }

  DataForm::DataForm( FormType type, const std::string& title )
    : StanzaExtension( ExtDataForm ),
      m_type( type ), m_title( title ), m_reported( 0 )
  {
  }

  DataForm::DataForm( const Tag* tag )
    : StanzaExtension( ExtDataForm ),
      m_type( TypeInvalid ), m_reported( 0 )
  {
    parse( tag );
  }

  DataForm::DataForm( const DataForm& form )
    : StanzaExtension( ExtDataForm ), DataFormFieldContainer( form ),
      m_type( form.m_type ), m_instructions( form.m_instructions ),
      m_title( form.m_title ), m_reported( form.m_reported ? new DataFormReported( form.m_reported->tag() ) : 0 )
  {
  }

  DataForm::~DataForm()
  {
    util::clearList( m_items );
    delete m_reported;
    m_reported = NULL;
  }

  static const char* dfTypeValues[] =
  {
    "form", "submit", "cancel", "result"
  };

  bool DataForm::parse( const Tag* tag )
  {
    if( !tag || tag->xmlns() != XMLNS_X_DATA || tag->name() != "x" )
      return false;

    const std::string& type = tag->findAttribute( TYPE );
    if( type.empty() )
      m_type = TypeForm;
    else
    {
      m_type = (FormType)util::lookup( type, dfTypeValues );
      if( m_type == TypeInvalid )
        return false;
    }

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "title" )
        m_title = (*it)->cdata();
      else if( (*it)->name() == "instructions" )
        m_instructions.push_back( (*it)->cdata() );
      else if( (*it)->name() == "field" )
        m_fields.push_back( new DataFormField( (*it) ) );
      else if( (*it)->name() == "reported" )
      {
        if( m_reported == NULL )
          m_reported = new DataFormReported( (*it) );
        // else - Invalid data form - only one "reported" is allowed
      }
      else if( (*it)->name() == "item" )
        m_items.push_back( new DataFormItem( (*it) ) );
    }

    return true;
  }

  const std::string& DataForm::filterString() const
  {
    static const std::string filter = "/message/x[@xmlns='" + XMLNS_X_DATA + "']";
    return filter;
  }

  Tag* DataForm::tag() const
  {
    if( m_type == TypeInvalid )
      return 0;

    Tag* x = new Tag( "x" );
    x->setXmlns( XMLNS_X_DATA );
    x->addAttribute( TYPE, util::lookup( m_type, dfTypeValues ) );
    if( !m_title.empty() )
      new Tag( x, "title", m_title );

    StringList::const_iterator it_i = m_instructions.begin();
    for( ; it_i != m_instructions.end(); ++it_i )
      new Tag( x, "instructions", (*it_i) );

    FieldList::const_iterator it = m_fields.begin();
    for( ; it != m_fields.end(); ++it )
      x->addChild( (*it)->tag() );

    if( m_reported != NULL )
    {
      x->addChild( m_reported->tag() );
    }

    ItemList::const_iterator iti = m_items.begin();
    for( ; iti != m_items.end(); ++iti )
      x->addChild( (*iti)->tag() );

    return x;
  }

}
