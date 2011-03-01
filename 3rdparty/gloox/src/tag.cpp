/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "tag.h"
#include "util.h"

#include <stdlib.h>

#include <algorithm>

namespace gloox
{

  // ---- Tag::Attribute ----
  Tag::Attribute::Attribute( Tag* parent, const std::string& name, const std::string& value,
                             const std::string& xmlns )
    : m_parent( parent )
  {
    if( m_parent )
      m_parent->addAttribute( this );

    init( name, value, xmlns );
  }

  Tag::Attribute::Attribute( const std::string& name, const std::string& value,
                             const std::string& xmlns )
    : m_parent( 0 )
  {
    init( name, value, xmlns );
  }

  Tag::Attribute::Attribute( const Attribute& attr )
    : m_parent( attr.m_parent ), m_name( attr.m_name ), m_value( attr.m_value ),
      m_xmlns( attr.m_xmlns ), m_prefix( attr.m_prefix )
  {
  }

  void Tag::Attribute::init( const std::string& name, const std::string& value,
                             const std::string& xmlns )
  {
    if( util::checkValidXMLChars( xmlns ) )
      m_xmlns = xmlns;
    else
      return;

    if( util::checkValidXMLChars( value ) )
      m_value = value;
    else
      return;

    if( util::checkValidXMLChars( name ) )
      m_name = name;
    else
      return;
  }

  bool Tag::Attribute::setValue( const std::string& value )
  {
    if( !util::checkValidXMLChars( value ) )
      return false;

    m_value = value;
    return true;
  }

  bool Tag::Attribute::setXmlns( const std::string& xmlns )
  {
    if( !util::checkValidXMLChars( xmlns ) )
      return false;

    m_xmlns = xmlns;
    return true;
  }

  bool Tag::Attribute::setPrefix( const std::string& prefix )
  {
    if( !util::checkValidXMLChars( prefix ) )
      return false;

    m_prefix = prefix;
    return true;
  }

  const std::string& Tag::Attribute::xmlns() const
  {
    if( !m_xmlns.empty() )
      return m_xmlns;

    if( m_parent )
      return m_parent->xmlns( m_prefix );

    return EmptyString;
  }

  const std::string& Tag::Attribute::prefix() const
  {
    if( !m_prefix.empty() )
      return m_prefix;

    if( m_parent )
      return m_parent->prefix( m_xmlns );

    return EmptyString;
  }

  const std::string Tag::Attribute::xml() const
  {
    if( m_name.empty() )
      return EmptyString;

    std::string xml;
    xml += ' ';
    if( !m_prefix.empty() )
    {
      xml += m_prefix;
      xml += ':';
    }
    xml += m_name;
    xml += "='";
    xml += util::escape( m_value );
    xml += '\'';

    return xml;
  }
  // ---- ~Tag::Attribute ----

  // ---- Tag ----
  Tag::Tag( const std::string& name, const std::string& cdata )
    : m_parent( 0 ), m_children( 0 ), m_cdata( 0 ),
      m_attribs( 0 ), m_nodes( 0 ),
      m_xmlnss( 0 )
  {
    addCData( cdata ); // implicitly UTF-8 checked

    if( util::checkValidXMLChars( name ) )
      m_name = name;
  }

  Tag::Tag( Tag* parent, const std::string& name, const std::string& cdata )
    : m_parent( parent ), m_children( 0 ), m_cdata( 0 ),
      m_attribs( 0 ), m_nodes( 0 ),
      m_xmlnss( 0 )
  {
    if( m_parent )
      m_parent->addChild( this );

    addCData( cdata ); // implicitly UTF-8 checked

    if( util::checkValidXMLChars( name ) )
      m_name = name;
  }

  Tag::Tag( const std::string& name,
            const std::string& attrib,
            const std::string& value )
    : m_parent( 0 ), m_children( 0 ), m_cdata( 0 ),
      m_attribs( 0 ), m_nodes( 0 ),
      m_name( name ), m_xmlnss( 0 )
  {
    addAttribute( attrib, value ); // implicitly UTF-8 checked

    if( util::checkValidXMLChars( name ) )
      m_name = name;
  }

  Tag::Tag( Tag* parent, const std::string& name,
                         const std::string& attrib,
                         const std::string& value )
    : m_parent( parent ), m_children( 0 ), m_cdata( 0 ),
      m_attribs( 0 ), m_nodes( 0 ),
      m_name( name ), m_xmlnss( 0 )
  {
    if( m_parent )
      m_parent->addChild( this );

    addAttribute( attrib, value ); // implicitly UTF-8 checked

    if( util::checkValidXMLChars( name ) )
      m_name = name;
  }

  Tag::Tag( Tag* tag )
    : m_parent( 0 ), m_children( 0 ), m_cdata( 0 ), m_attribs( 0 ),
      m_nodes( 0 ), m_xmlnss( 0 )
  {
    if( !tag )
      return;

    m_children = tag->m_children;
    m_cdata = tag->m_cdata;
    m_attribs = tag->m_attribs;
    m_nodes = tag->m_nodes;
    m_name = tag->m_name;
    m_xmlns = tag->m_xmlns;
    m_xmlnss = tag->m_xmlnss;

    tag->m_nodes = 0;
    tag->m_cdata = 0;
    tag->m_attribs = 0;
    tag->m_children = 0;
    tag->m_xmlnss = 0;

    if( m_attribs )
    {
      AttributeList::iterator it = m_attribs->begin();
      while( it != m_attribs->end() )
        (*it++)->m_parent = this;
    }

    if( m_children )
    {
      TagList::iterator it = m_children->begin();
      while( it != m_children->end() )
        (*it++)->m_parent = this;
    }
  }

  Tag::~Tag()
  {
    if( m_cdata )
      util::clearList( *m_cdata );
    if( m_attribs )
      util::clearList( *m_attribs );
    if( m_children )
      util::clearList( *m_children );
    if( m_nodes )
      util::clearList( *m_nodes );

    delete m_cdata;
    delete m_attribs;
    delete m_children;
    delete m_nodes;
    delete m_xmlnss;

    m_parent = 0;
  }

  bool Tag::operator==( const Tag& right ) const
  {
    if( m_name != right.m_name || m_xmlns != right.m_xmlns )
      return false;

    if( m_cdata && right.m_cdata )
    {
      StringPList::const_iterator ct = m_cdata->begin();
      StringPList::const_iterator ct_r = right.m_cdata->begin();
      while( ct != m_cdata->end() && ct_r != right.m_cdata->end() && *(*ct) == *(*ct_r) )
      {
        ++ct;
        ++ct_r;
      }
      if( ct != m_cdata->end() )
        return false;
    }
    else if( m_cdata || right.m_cdata )
      return false;

    if( m_children && right.m_children )
    {
      TagList::const_iterator it = m_children->begin();
      TagList::const_iterator it_r = right.m_children->begin();
      while( it != m_children->end() && it_r != right.m_children->end() && *(*it) == *(*it_r) )
      {
        ++it;
        ++it_r;
      }
      if( it != m_children->end() )
        return false;
    }
    else if( m_children || right.m_children )
      return false;

    if( m_attribs && right.m_attribs )
    {
      AttributeList::const_iterator at = m_attribs->begin();
      AttributeList::const_iterator at_r = right.m_attribs->begin();
      while( at != m_attribs->end() && at_r != right.m_attribs->end() && *(*at) == *(*at_r) )
      {
        ++at;
        ++at_r;
      }
      if( at != m_attribs->end() )
        return false;
    }
    else if( m_attribs || right.m_attribs )
      return false;

    return true;
  }

  const std::string Tag::xml() const
  {
    if( m_name.empty() )
      return EmptyString;

    std::string xml = "<";
    if( !m_prefix.empty() )
    {
      xml += m_prefix;
      xml += ':';
    }
    xml += m_name;
    if( m_attribs && !m_attribs->empty() )
    {
      AttributeList::const_iterator it_a = m_attribs->begin();
      for( ; it_a != m_attribs->end(); ++it_a )
      {
        xml += (*it_a)->xml();
      }
    }

    if( !m_nodes || m_nodes->empty() )
      xml += "/>";
    else
    {
      xml += '>';
      NodeList::const_iterator it_n = m_nodes->begin();
      for( ; it_n != m_nodes->end(); ++it_n )
      {
        switch( (*it_n)->type )
        {
          case TypeTag:
            xml += (*it_n)->tag->xml();
            break;
          case TypeString:
            xml += util::escape( *((*it_n)->str) );
            break;
        }
      }
      xml += "</";
      if( !m_prefix.empty() )
      {
        xml += m_prefix;
        xml += ':';
      }
      xml += m_name;
      xml += '>';
    }

    return xml;
  }

  bool Tag::addAttribute( Attribute* attr )
  {
    if( !attr )
      return false;

    if( !(*attr) )
    {
      delete attr;
      return false;
    }

    if( !m_attribs )
      m_attribs = new AttributeList();

    AttributeList::iterator it = m_attribs->begin();
    for( ; it != m_attribs->end(); ++it )
    {
      if( (*it)->name() == attr->name()
          && ( (*it)->xmlns() == attr->xmlns() || (*it)->prefix() == attr->prefix() ) )
      {
        delete (*it);
        (*it) = attr;
        return true;
      }
    }

    m_attribs->push_back( attr );

    return true;
  }

  bool Tag::addAttribute( const std::string& name, const std::string& value )
  {
    if( name.empty() || value.empty() )
      return false;

    return addAttribute( new Attribute( name, value ) );
  }

  bool Tag::addAttribute( const std::string& name, int value )
  {
    if( name.empty() )
      return false;

    return addAttribute( name, util::int2string( value ) );
  }

  bool Tag::addAttribute( const std::string& name, long value )
  {
    if( name.empty() )
      return false;

    return addAttribute( name, util::long2string( value ) );
  }

  void Tag::setAttributes( const AttributeList& attributes )
  {
    if( !m_attribs )
      m_attribs = new AttributeList( attributes );
    else
    {
      util::clearList( *m_attribs );
      *m_attribs = attributes;
    }

    AttributeList::iterator it = m_attribs->begin();
    for( ; it != m_attribs->end(); ++it )
      (*it)->m_parent = this;
  }

  void Tag::addChild( Tag* child )
  {
    if( !child )
      return;

    if( !m_nodes )
      m_nodes = new NodeList();
    if( !m_children )
      m_children = new TagList();

    m_children->push_back( child );
    child->m_parent = this;
    m_nodes->push_back( new Node( TypeTag, child ) );
  }

  void Tag::addChildCopy( const Tag* child )
  {
    if( !child )
      return;

    addChild( child->clone() );
  }

  bool Tag::setCData( const std::string& cdata )
  {
    if( cdata.empty() || !util::checkValidXMLChars( cdata ) )
      return false;

    if( !m_cdata )
      m_cdata = new StringPList();
    else
      util::clearList( *m_cdata );

    if( !m_nodes )
      m_nodes = new NodeList();
    else
    {
      NodeList::iterator it = m_nodes->begin();
      NodeList::iterator t;
      while( it != m_nodes->end() )
      {
        if( (*it)->type == TypeString )
        {
          t = it++;
          delete (*t);
          m_nodes->erase( t );
        }
      }
    }

    return addCData( cdata );
  }

  bool Tag::addCData( const std::string& cdata )
  {
    if( cdata.empty() || !util::checkValidXMLChars( cdata ) )
      return false;

    if( !m_cdata )
      m_cdata = new StringPList();
    if( !m_nodes )
      m_nodes = new NodeList();

    std::string* str = new std::string( cdata );
    m_cdata->push_back( str );
    m_nodes->push_back( new Node( TypeString, str ) );
    return true;
  }

  const std::string Tag::cdata() const
  {
    if( !m_cdata )
      return EmptyString;

    std::string str;
    StringPList::const_iterator it = m_cdata->begin();
    for( ; it != m_cdata->end(); ++it )
      str += *(*it);

    return str;
  }

  const TagList& Tag::children() const
  {
    static const TagList empty;
    return m_children ? *m_children : empty;
  }

  const Tag::AttributeList& Tag::attributes() const
  {
    static const AttributeList empty;
    return m_attribs ? *m_attribs : empty;
  }

  bool Tag::setXmlns( const std::string& xmlns, const std::string& prefix )
  {
    if( !util::checkValidXMLChars( xmlns ) || !util::checkValidXMLChars( prefix ) )
      return false;

    if( prefix.empty() )
    {
      m_xmlns = xmlns;
      return addAttribute( XMLNS, m_xmlns );
    }
    else
    {
      if( !m_xmlnss )
        m_xmlnss = new StringMap();

      (*m_xmlnss)[prefix] = xmlns;

      return addAttribute( XMLNS + ":" + prefix, xmlns );
    }
  }

  const std::string& Tag::xmlns() const
  {
    return xmlns( m_prefix );
  }

  const std::string& Tag::xmlns( const std::string& prefix ) const
  {
    if( prefix.empty() )
    {
      return hasAttribute( XMLNS ) ? findAttribute( XMLNS ) : m_xmlns;
    }

    if( m_xmlnss )
    {
      StringMap::const_iterator it = m_xmlnss->find( prefix );
      if( it != m_xmlnss->end() )
        return (*it).second;
    }

    return m_parent ? m_parent->xmlns( prefix ) : EmptyString;
  }

  bool Tag::setPrefix( const std::string& prefix )
  {
    if( !util::checkValidXMLChars( prefix ) )
      return false;

    m_prefix = prefix;
    return true;
  }

  const std::string& Tag::prefix( const std::string& xmlns ) const
  {
    if( xmlns.empty() || !m_xmlnss )
      return EmptyString;

    StringMap::const_iterator it = m_xmlnss->begin();
    for( ; it != m_xmlnss->end(); ++it )
    {
      if( (*it).second == xmlns )
        return (*it).first;
    }

    return EmptyString;
  }

  const std::string& Tag::findAttribute( const std::string& name ) const
  {
    if( !m_attribs )
      return EmptyString;

    AttributeList::const_iterator it = m_attribs->begin();
    for( ; it != m_attribs->end(); ++it )
      if( (*it)->name() == name )
        return (*it)->value();

    return EmptyString;
  }

  bool Tag::hasAttribute( const std::string& name, const std::string& value ) const
  {
    if( name.empty() || !m_attribs )
      return false;

    AttributeList::const_iterator it = m_attribs->begin();
    for( ; it != m_attribs->end(); ++it )
      if( (*it)->name() == name )
        return value.empty() || (*it)->value() == value;

    return false;
  }

  bool Tag::hasChild( const std::string& name, const std::string& attr,
                      const std::string& value ) const
  {
    if( attr.empty() )
      return findChild( name ) ? true : false;
    else
      return findChild( name, attr, value ) ? true : false;
  }

  Tag* Tag::findChild( const std::string& name ) const
  {
    if( !m_children )
      return 0;

    TagList::const_iterator it = m_children->begin();
    while( it != m_children->end() && (*it)->name() != name )
      ++it;
    return it != m_children->end() ? (*it) : 0;
  }

  Tag* Tag::findChild( const std::string& name, const std::string& attr,
                       const std::string& value ) const
  {
    if( !m_children || name.empty() )
      return 0;

    TagList::const_iterator it = m_children->begin();
    while( it != m_children->end() && ( (*it)->name() != name || !(*it)->hasAttribute( attr, value ) ) )
      ++it;
    return it != m_children->end() ? (*it) : 0;
  }

  bool Tag::hasChildWithCData( const std::string& name, const std::string& cdata ) const
  {
    if( !m_children || name.empty() || cdata.empty() )
      return 0;

    TagList::const_iterator it = m_children->begin();
    while( it != m_children->end() && ( (*it)->name() != name
            || ( !cdata.empty() && (*it)->cdata() != cdata ) ) )
      ++it;
    return it != m_children->end();
  }

  Tag* Tag::findChildWithAttrib( const std::string& attr, const std::string& value ) const
  {
    if( !m_children || attr.empty() )
      return 0;

    TagList::const_iterator it = m_children->begin();
    while( it != m_children->end() && !(*it)->hasAttribute( attr, value ) )
      ++it;
    return it != m_children->end() ? (*it) : 0;
  }

  Tag* Tag::clone() const
  {
    Tag* t = new Tag( m_name );
    t->m_xmlns = m_xmlns;
    t->m_prefix = m_prefix;

    if( m_attribs )
    {
      t->m_attribs = new AttributeList();
      Tag::AttributeList::const_iterator at = m_attribs->begin();
      Attribute* attr;
      for( ; at != m_attribs->end(); ++at )
      {
        attr = new Attribute( *(*at) );
        attr->m_parent = t;
        t->m_attribs->push_back( attr );
      }
    }

    if( m_xmlnss )
    {
      t->m_xmlnss = new StringMap( *m_xmlnss );
    }

    if( m_nodes )
    {
      Tag::NodeList::const_iterator nt = m_nodes->begin();
      for( ; nt != m_nodes->end(); ++nt )
      {
        switch( (*nt)->type )
        {
          case TypeTag:
            t->addChild( (*nt)->tag->clone() );
            break;
          case TypeString:
            t->addCData( *((*nt)->str) );
            break;
        }
      }
    }

    return t;
  }

  TagList Tag::findChildren( const std::string& name,
                             const std::string& xmlns ) const
  {
    return m_children ? findChildren( *m_children, name, xmlns ) : TagList();
  }

  TagList Tag::findChildren( const TagList& list, const std::string& name,
                             const std::string& xmlns ) const
  {
    TagList ret;
    TagList::const_iterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
      if( (*it)->name() == name && ( xmlns.empty() || (*it)->xmlns() == xmlns ) )
        ret.push_back( (*it) );
    }
    return ret;
  }

  void Tag::removeChild( const std::string& name, const std::string& xmlns )
  {
    if( name.empty() || !m_children || !m_nodes )
      return;

    TagList l = findChildren( name, xmlns );
    TagList::iterator it = l.begin();
    TagList::iterator it2;
    while( it != l.end() )
    {
      it2 = it++;
      NodeList::iterator itn = m_nodes->begin();
      for( ; itn != m_nodes->end(); ++itn )
      {
        if( (*itn)->type == TypeTag && (*itn)->tag == (*it2) )
        {
          delete (*itn);
          m_nodes->erase( itn );
          break;
        }
      }
      m_children->remove( (*it2) );
      delete (*it2);
    }
  }

  void Tag::removeChild( Tag* tag )
  {
    if( m_children )
      m_children->remove( tag );

    if( !m_nodes )
      return;

    NodeList::iterator it = m_nodes->begin();
    for( ; it != m_nodes->end(); ++it )
    {
      if( (*it)->type == TypeTag && (*it)->tag == tag )
      {
        delete (*it);
        m_nodes->erase( it );
        return;
      }
    }
  }

  void Tag::removeAttribute( const std::string& attr, const std::string& value,
                             const std::string& xmlns )
  {
    if( attr.empty() || !m_attribs )
      return;

    AttributeList::iterator it = m_attribs->begin();
    AttributeList::iterator it2;
    while( it != m_attribs->end() )
    {
      it2 = it++;
      if( (*it2)->name() == attr && ( value.empty() || (*it2)->value() == value )
                                 && ( xmlns.empty() || (*it2)->xmlns() == xmlns ) )
      {
        delete (*it2);
        m_attribs->erase( it2 );
      }
    }
  }

  const std::string Tag::findCData( const std::string& expression ) const
  {
    const ConstTagList& l = findTagList( expression );
    return !l.empty() ? l.front()->cdata() : EmptyString;
  }

  const Tag* Tag::findTag( const std::string& expression ) const
  {
    const ConstTagList& l = findTagList( expression );
    return !l.empty() ? l.front() : 0;
  }

  ConstTagList Tag::findTagList( const std::string& expression ) const
  {
    ConstTagList l;
    if( expression == "/" || expression == "//" )
      return l;

    if( m_parent && expression.length() >= 2 && expression[0] == '/'
                                             && expression[1] != '/' )
      return m_parent->findTagList( expression );

    unsigned len = 0;
    Tag* p = parse( expression, len );
//     if( p )
//       printf( "parsed tree: %s\n", p->xml().c_str() );
    l = evaluateTagList( p );
    delete p;
    return l;
  }

  ConstTagList Tag::evaluateTagList( Tag* token ) const
  {
    ConstTagList result;
    if( !token )
      return result;

//     printf( "evaluateTagList called in Tag %s and Token %s (type: %s)\n", name().c_str(),
//             token->name().c_str(), token->findAttribute( TYPE ).c_str() );

    TokenType tokenType = (TokenType)atoi( token->findAttribute( TYPE ).c_str() );
    switch( tokenType )
    {
      case XTUnion:
        add( result, evaluateUnion( token ) );
        break;
      case XTElement:
      {
//         printf( "in XTElement, token: %s\n", token->name().c_str() );
        if( token->name() == name() || token->name() == "*" )
        {
//           printf( "found %s\n", name().c_str() );
          const TagList& tokenChildren = token->children();
          if( tokenChildren.size() )
          {
            bool predicatesSucceeded = true;
            TagList::const_iterator cit = tokenChildren.begin();
            for( ; cit != tokenChildren.end(); ++cit )
            {
              if( (*cit)->hasAttribute( "predicate", "true" ) )
              {
                predicatesSucceeded = evaluatePredicate( (*cit) );
                if( !predicatesSucceeded )
                  return result;
              }
            }

            bool hasElementChildren = false;
            cit = tokenChildren.begin();
            for( ; cit != tokenChildren.end(); ++cit )
            {
              if( (*cit)->hasAttribute( "predicate", "true" ) ||
                  (*cit)->hasAttribute( "number", "true" ) )
                continue;

              hasElementChildren = true;

//               printf( "checking %d children of token %s\n", tokenChildren.size(), token->name().c_str() );
              if( m_children && !m_children->empty() )
              {
                TagList::const_iterator it = m_children->begin();
                for( ; it != m_children->end(); ++it )
                {
                  add( result, (*it)->evaluateTagList( (*cit) ) );
                }
              }
              else if( atoi( (*cit)->findAttribute( TYPE ).c_str() ) == XTDoubleDot && m_parent )
              {
                (*cit)->addAttribute( TYPE, XTDot );
                add( result, m_parent->evaluateTagList( (*cit) ) );
              }
            }

            if( !hasElementChildren )
              result.push_back( this );
          }
          else
          {
//             printf( "adding %s to result set\n", name().c_str() );
            result.push_back( this );
          }
        }
//         else
//           printf( "found %s != %s\n", token->name().c_str(), name().c_str() );

        break;
      }
      case XTDoubleSlash:
      {
//         printf( "in XTDoubleSlash\n" );
        Tag* t = token->clone();
//         printf( "original token: %s\ncloned token: %s\n", token->xml().c_str(), n->xml().c_str() );
        t->addAttribute( TYPE, XTElement );
        add( result, evaluateTagList( t ) );
        const ConstTagList& res2 = allDescendants();
        ConstTagList::const_iterator it = res2.begin();
        for( ; it != res2.end(); ++it )
        {
          add( result, (*it)->evaluateTagList( t ) );
        }
        delete t;
        break;
      }
      case XTDot:
      {
        const TagList& tokenChildren = token->children();
        if( !tokenChildren.empty() )
        {
          add( result, evaluateTagList( tokenChildren.front() ) );
        }
        else
          result.push_back( this );
        break;
      }
      case XTDoubleDot:
      {
//         printf( "in XTDoubleDot\n" );
        if( m_parent )
        {
          const TagList& tokenChildren = token->children();
          if( tokenChildren.size() )
          {
            Tag* testtoken = tokenChildren.front();
            if( testtoken->name() == "*" )
            {
              add( result, m_parent->evaluateTagList( testtoken ) );
            }
            else
            {
              Tag* t = token->clone();
              t->addAttribute( TYPE, XTElement );
              t->m_name = m_parent->m_name;
              add( result, m_parent->evaluateTagList( t ) );
              delete t;
            }
          }
          else
          {
            result.push_back( m_parent );
          }
        }
      }
      case XTInteger:
      {
        const TagList& l = token->children();
        if( !l.size() )
          break;

        const ConstTagList& res = evaluateTagList( l.front() );

        int pos = atoi( token->name().c_str() );
//         printf( "checking index %d\n", pos );
        if( pos > 0 && pos <= (int)res.size() )
        {
          ConstTagList::const_iterator it = res.begin();
          while ( --pos )
          {
            ++it;
          }
          result.push_back( *it );
        }
        break;
      }
      default:
        break;
    }
    return result;
  }

  bool Tag::evaluateBoolean( Tag* token ) const
  {
    if( !token )
      return false;

    bool result = false;
    TokenType tokenType = (TokenType)atoi( token->findAttribute( TYPE ).c_str() );
    switch( tokenType )
    {
      case XTAttribute:
        if( token->name() == "*" && m_attribs && m_attribs->size() )
          result = true;
        else
          result = hasAttribute( token->name() );
        break;
      case XTOperatorEq:
        result = evaluateEquals( token );
        break;
      case XTOperatorLt:
        break;
      case XTOperatorLtEq:
        break;
      case XTOperatorGtEq:
        break;
      case XTOperatorGt:
        break;
      case XTUnion:
      case XTElement:
      {
        Tag* t = new Tag( "." );
        t->addAttribute( TYPE, XTDot );
        t->addChild( token );
        result = !evaluateTagList( t ).empty();
        t->removeChild( token );
        delete t;
        break;
      }
      default:
        break;
    }

    return result;
  }

  bool Tag::evaluateEquals( Tag* token ) const
  {
    if( !token || token->children().size() != 2 )
      return false;

    bool result = false;
    TagList::const_iterator it = token->children().begin();
    Tag* ch1 = (*it);
    Tag* ch2 = (*++it);

    TokenType tt1 = (TokenType)atoi( ch1->findAttribute( TYPE ).c_str() );
    TokenType tt2 = (TokenType)atoi( ch2->findAttribute( TYPE ).c_str() );
    switch( tt1 )
    {
      case XTAttribute:
        switch( tt2 )
        {
          case XTInteger:
          case XTLiteral:
            result = ( findAttribute( ch1->name() ) == ch2->name() );
            break;
          case XTAttribute:
            result = ( hasAttribute( ch1->name() ) && hasAttribute( ch2->name() ) &&
                      findAttribute( ch1->name() ) == findAttribute( ch2->name() ) );
            break;
          default:
            break;
        }
        break;
      case XTInteger:
      case XTLiteral:
        switch( tt2 )
        {
          case XTAttribute:
            result = ( ch1->name() == findAttribute( ch2->name() ) );
            break;
          case XTLiteral:
          case XTInteger:
            result = ( ch1->name() == ch2->name() );
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }

    return result;
  }

  ConstTagList Tag::allDescendants() const
  {
    ConstTagList result;

    if( !m_children )
      return result;

    TagList::const_iterator it = m_children->begin();
    for( ; it != m_children->end(); ++it )
    {
      result.push_back( (*it) );
      add( result, (*it)->allDescendants() );
    }
    return result;
  }

  ConstTagList Tag::evaluateUnion( Tag* token ) const
  {
    ConstTagList result;
    if( !token )
      return result;

    const TagList& l = token->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      add( result, evaluateTagList( (*it) ) );
    }
    return result;
  }

  void Tag::closePreviousToken( Tag** root, Tag** current, Tag::TokenType& type, std::string& tok ) const
  {
    if( !tok.empty() )
    {
      addToken( root, current, type, tok );
      type = XTElement;
      tok = EmptyString;
    }
  }

  Tag* Tag::parse( const std::string& expression, unsigned& len, Tag::TokenType border ) const
  {
    Tag* root = 0;
    Tag* current = root;
    std::string token;

//     XPathError error = XPNoError;
//     XPathState state = Init;
//     int expected = 0;
//     bool run = true;
//     bool ws = false;

    Tag::TokenType type  = XTElement;

    char c;
    for( ; len < expression.length(); ++len )
    {
      c = expression[len];
      if( type == XTLiteralInside && c != '\'' )
      {
        token += c;
        continue;
      }

      switch( c )
      {
        case '/':
          closePreviousToken( &root, &current, type, token );

          if( len < expression.length()-1 && expression[len+1] == '/' )
          {
//             addToken( &root, &current, XTDoubleSlash, "//" );
            type = XTDoubleSlash;
            ++len;
          }
//           else
//           {
//             if( !current )
//             addToken( &root, &current, XTSlash, "/" );
//           }
          break;
        case ']':
          closePreviousToken( &root, &current, type, token );
          return root;
        case '[':
        {
          closePreviousToken( &root, &current, type, token );
          Tag* t = parse( expression, ++len, XTRightBracket );
          if( !addPredicate( &root, &current, t ) )
            delete t;
          break;
        }
        case '(':
        {
          closePreviousToken( &root, &current, type, token );
          Tag* t = parse( expression, ++len, XTRightParenthesis );
          if( current )
          {
//             printf( "added %s to %s\n", t->xml().c_str(), current->xml().c_str() );
            t->addAttribute( "argument", "true" );
            current->addChild( t );
          }
          else
          {
            root = t;
//             printf( "made %s new root\n", t->xml().c_str() );
          }
          break;
        }
        case ')':
          closePreviousToken( &root, &current, type, token );
          ++len;
          return root;
        case '\'':
          if( type == XTLiteralInside )
            if( expression[len - 2] == '\\' )
              token[token.length() - 2] = c;
            else
              type = XTLiteral;
          else
            type = XTLiteralInside;
          break;
        case '@':
          type = XTAttribute;
          break;
        case '.':
          token += c;
          if( token.size() == 1 )
          {
            if( len < expression.length()-1 && expression[len+1] == '.' )
            {
              type = XTDoubleDot;
              ++len;
              token += c;
            }
            else
            {
              type = XTDot;
            }
          }
          break;
        case '*':
//           if( !root || ( current && ( current->tokenType() == XTSlash
//                                       || current->tokenType() == XTDoubleSlash ) ) )
//           {
//             addToken( &root, &current, type, "*" );
//             break;
//           }
          addToken( &root, &current, type, "*" );
          type = XTElement;
          break;
        case '+':
        case '>':
        case '<':
        case '=':
        case '|':
        {
          closePreviousToken( &root, &current, type, token );
          std::string s( 1, c );
          Tag::TokenType ttype = getType( s );
          if( ttype <= border )
            return root;
          Tag* t = parse( expression, ++len, ttype );
          addOperator( &root, &current, t, ttype, s );
          if( border == XTRightBracket )
            return root;
          break;
        }
        default:
          token += c;
      }
    }

    if( !token.empty() )
      addToken( &root, &current, type, token );

//     if( error != XPNoError )
//       printf( "error: %d\n", error );
    return root;
  }

  void Tag::addToken( Tag **root, Tag **current, Tag::TokenType type,
                      const std::string& token ) const
  {
    Tag* t = new Tag( token );
    if( t->isNumber() && !t->children().size() )
      type = XTInteger;
    t->addAttribute( TYPE, type );

    if( *root )
    {
//       printf( "new current %s, type: %d\n", token.c_str(), type );
      (*current)->addChild( t );
      *current = t;
    }
    else
    {
//       printf( "new root %s, type: %d\n", token.c_str(), type );
      *current = *root = t;
    }
  }

  void Tag::addOperator( Tag** root, Tag** current, Tag* arg,
                           Tag::TokenType type, const std::string& token ) const
  {
    Tag* t = new Tag( token );
    t->addAttribute( TYPE, type );
//     printf( "new operator: %s (arg1: %s, arg2: %s)\n", t->name().c_str(), (*root)->xml().c_str(),
//                                                                           arg->xml().c_str() );
    t->addAttribute( "operator", "true" );
    t->addChild( *root );
    t->addChild( arg );
    *current = *root = t;
  }

  bool Tag::addPredicate( Tag **root, Tag **current, Tag* token ) const
  {
    if( !*root || !*current )
      return false;

    if( ( token->isNumber() && !token->children().size() ) || token->name() == "+" )
    {
//       printf( "found Index %s, full: %s\n", token->name().c_str(), token->xml().c_str() );
      if( !token->hasAttribute( "operator", "true" ) )
      {
        token->addAttribute( TYPE, XTInteger );
      }
      if( *root == *current )
      {
        *root = token;
//         printf( "made Index new root\n" );
      }
      else
      {
        (*root)->removeChild( *current );
        (*root)->addChild( token );
//         printf( "added Index somewhere between root and current\n" );
      }
      token->addChild( *current );
//       printf( "added Index %s, full: %s\n", token->name().c_str(), token->xml().c_str() );
    }
    else
    {
      token->addAttribute( "predicate", "true" );
      (*current)->addChild( token );
    }

    return true;
  }

  Tag::TokenType Tag::getType( const std::string& c )
  {
    if( c == "|" )
      return XTUnion;
    if( c == "<" )
      return XTOperatorLt;
    if( c == ">" )
      return XTOperatorGt;
    if( c == "*" )
      return XTOperatorMul;
    if( c == "+" )
      return XTOperatorPlus;
    if( c == "=" )
      return XTOperatorEq;

    return XTNone;
  }

  bool Tag::isWhitespace( const char c )
  {
    return ( c == 0x09 || c == 0x0a || c == 0x0d || c == 0x20 );
  }

  bool Tag::isNumber() const
  {
    if( m_name.empty() )
      return false;

    std::string::size_type l = m_name.length();
    std::string::size_type i = 0;
    while( i < l && isdigit( m_name[i] ) )
      ++i;
    return i == l;
  }

  void Tag::add( ConstTagList& one, const ConstTagList& two )
  {
    ConstTagList::const_iterator it = two.begin();
    for( ; it != two.end(); ++it )
      if( std::find( one.begin(), one.end(), (*it) ) == one.end() )
        one.push_back( (*it) );
  }

}
