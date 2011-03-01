/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "dataformfield.h"
#include "util.h"
#include "tag.h"

namespace gloox
{

  static const char* fieldTypeValues[] =
  {
    "boolean", "fixed", "hidden", "jid-multi", "jid-single",
    "list-multi", "list-single", "text-multi", "text-private", "text-single", ""
  };

  DataFormField::DataFormField( FieldType type )
    : m_type( type ), m_required( false )
  {
  }

  DataFormField::DataFormField( const std::string& name, const std::string& value,
                        const std::string& label, FieldType type )
    : m_type( type ), m_name( name ), m_label( label ), m_required( false )
  {
    m_values.push_back( value );
  }

  DataFormField::DataFormField( const Tag* tag )
    : m_type( TypeInvalid ), m_required( false )
  {
    if( !tag )
      return;

    const std::string& type = tag->findAttribute( TYPE );
    if( type.empty() )
    {
      if( !tag->name().empty() )
        m_type = TypeNone;
    }
    else
      m_type = (FieldType)util::lookup( type, fieldTypeValues );

    if( tag->hasAttribute( "var" ) )
      m_name = tag->findAttribute( "var" );

    if( tag->hasAttribute( "label" ) )
      m_label = tag->findAttribute( "label" );

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "desc" )
        m_desc = (*it)->cdata();
      else if( (*it)->name() == "required" )
        m_required = true;
      else if( (*it)->name() == "value" )
      {
        if( m_type == TypeTextMulti || m_type == TypeListMulti || m_type == TypeJidMulti )
          addValue( (*it)->cdata() );
        else
          setValue( (*it)->cdata() );
      }
      else if( (*it)->name() == "option" )
      {
        Tag* v = (*it)->findChild( "value" );
        if( v )
          m_options.insert( std::make_pair( (*it)->findAttribute( "label" ), v->cdata() ) );
      }
    }

  }

  DataFormField::~DataFormField()
  {
  }

  Tag* DataFormField::tag() const
  {
    if( m_type == TypeInvalid )
      return 0;

    Tag* field = new Tag( "field" );
    field->addAttribute( TYPE, util::lookup( m_type, fieldTypeValues ) );
    field->addAttribute( "var", m_name );
    field->addAttribute( "label", m_label );
    if( m_required )
      new Tag( field, "required" );

    if( !m_desc.empty() )
      new Tag( field, "desc", m_desc );

    if( m_type == TypeListSingle || m_type == TypeListMulti )
    {
      StringMultiMap::const_iterator it = m_options.begin();
      for( ; it != m_options.end(); ++it )
      {
        Tag* option = new Tag( field, "option", "label", (*it).first );
        new Tag( option, "value", (*it).second );
      }
    }
    else if( m_type == TypeBoolean )
    {
      if( m_values.size() == 0 || m_values.front() == "false" || m_values.front() == "0" )
        new Tag( field, "value", "0" );
      else
        new Tag( field, "value", "1" );
    }

    if( m_type == TypeTextMulti || m_type == TypeListMulti || m_type == TypeJidMulti )
    {
      StringList::const_iterator it = m_values.begin();
      for( ; it != m_values.end() ; ++it )
        new Tag( field, "value", (*it) );
    }

    if( m_values.size() && !( m_type == TypeTextMulti || m_type == TypeListMulti
                              || m_type == TypeBoolean || m_type == TypeJidMulti ) )
      new Tag( field, "value", m_values.front() );

    return field;
  }

}
