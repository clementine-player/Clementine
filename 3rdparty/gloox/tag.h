/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef TAG_H__
#define TAG_H__

#include "gloox.h"

#include <string>
#include <list>
#include <utility>

namespace gloox
{

  class Tag;

  /**
   * A list of Tags.
   */
  typedef std::list<Tag*> TagList;

  /**
   * A list of const Tags.
   */
  typedef std::list<const Tag*> ConstTagList;

  /**
   * @brief This is an abstraction of an XML element.
   *
   * @note Use setXmlns() to set namespaces and namespace prefixes.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.4
   */
  class GLOOX_API Tag
  {

    friend class Parser;

    public:

      /**
       * An XML element's attribute.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class GLOOX_API Attribute
      {

        friend class Tag;

        public:
          /**
           * Creates a new Attribute from @c name, @c value and optional @c xmlns and attaches
           * it to the given Tag.
           *
           * In the future: If @c xmlns is not empty, and if it is different from the Tag's
           * default namespace, an appropriate and unique namespace declaration (prefix) will
           * be added to the Tag and the attribute will be prefixed accordingly.
           * @param parent The Tag to attach the Attribute to.
           * @param name The attribute's name. Invalid (non-UTF-8) input will be ignored.
           * @param value The attribute's value. Invalid (non-UTF-8) input will be ignored.
           * @param xmlns The attribute's namespace. Invalid (non-UTF-8) input will be ignored.
           */
          Attribute( Tag* parent, const std::string& name, const std::string& value,
                     const std::string& xmlns = EmptyString );

          /**
           * Creates a new Attribute from @c name, @c value and optional @c xmlns.
           * @param name The attribute's name. Invalid (non-UTF-8) input will be ignored.
           * @param value The attribute's value. Invalid (non-UTF-8) input will be ignored.
           * @param xmlns The attribute's namespace. Invalid (non-UTF-8) input will be ignored.
           */
          Attribute( const std::string& name, const std::string& value,
                     const std::string& xmlns = EmptyString );

          /**
           * Copy constructor.
           * @param attr The Attribute to copy.
           */
          Attribute( const Attribute& attr );

          /**
           * Destructor.
           */
          virtual ~Attribute() {}

          /**
           * Returns the attribute's name.
           * @return The attribute's name.
           */
          const std::string& name() const { return m_name; }

          /**
           * Returns the attribute's value.
           * @return The attribute's value.
           */
          const std::string& value() const { return m_value; }

          /**
           * Sets the attribute's value.
           * @param value The new value.
           * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
           * input will be ignored.
           */
          bool setValue( const std::string& value );

          /**
           * Returns the attribute's namespace.
           * @return The attribute's namespace.
           */
          const std::string& xmlns() const;

          /**
           * Sets the attribute's namespace.
           * @param xmlns The new namespace.
           * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
           * input will be ignored.
           */
          bool setXmlns( const std::string& xmlns );

          /**
           * Sets the attribute's namespace prefix.
           * @param prefix The new namespace prefix.
           * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
           * input will be ignored.
           */
          bool setPrefix( const std::string& prefix );

          /**
           * Returns the attribute's namespace prefix.
           * @return The namespace prefix.
           */
          const std::string& prefix() const;

          /**
           * Returns a string representation of the attribute.
           * @return A string representation.
           */
          const std::string xml() const;

          /**
           * Checks two Attributes for equality.
           * @param right The Attribute to check against the current Attribute.
           */
          bool operator==( const Attribute &right ) const
            { return m_name == right.m_name && m_value == right.m_value && m_xmlns == right.m_xmlns; }

          /**
           * Checks two Attributes for inequality.
           * @param right The Attribute to check against the current Attribute.
           */
          bool operator!=( const Attribute &right ) const
            { return !( *this == right ); }

          /**
           * Returns @b true if the Attribute is valid, @b false otherwise.
           */
          operator bool() const { return !m_name.empty(); }

        private:
          void init( const std::string& name, const std::string& value,
                     const std::string& xmlns );
          Tag* m_parent;
          std::string m_name;
          std::string m_value;
          std::string m_xmlns;
          std::string m_prefix;

      };

      /**
       * A list of XML element attributes.
       */
      typedef std::list<Attribute*> AttributeList;

      /**
       * Creates a new tag with a given name (and XML character data, if given).
       * @param name The name of the element.
       * @param cdata The XML character data of the element.
       */
      Tag( const std::string& name, const std::string& cdata = EmptyString );

      /**
       * Creates a new tag as a child tag of the given parent, with a given name (and
       * XML character data, if given).
       * @param parent The parent tag.
       * @param name The name of the element.
       * @param cdata The XML character data of the element.
       */
      Tag( Tag* parent, const std::string& name, const std::string& cdata = EmptyString );

      /**
       * Creates a new tag with a given name and an attribute.
       * @param name The name of the element.
       * @param attrib The attribute name.
       * @param value The attribute value.
       */
      Tag( const std::string& name, const std::string& attrib, const std::string& value );

      /**
       * Creates a new tag as a child tag of the given parent, with a given name and
       * an attribute.
       * @param parent The parent tag.
       * @param name The name of the element.
       * @param attrib The attribute name.
       * @param value The attribute value.
       */
      Tag( Tag* parent, const std::string& name, const std::string& attrib, const std::string& value );

      /**
       * Virtual destructor.
       */
      virtual ~Tag();

      /**
       * This function can be used to retrieve the complete XML of a tag as a string.
       * It includes all the attributes, child nodes and character data.
       * @return The complete XML.
       */
      const std::string xml() const;

      /**
       * Sets the Tag's namespace prefix.
       * @param prefix The namespace prefix.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       * @since 1.0
       */
      bool setPrefix( const std::string& prefix );

      /**
       * Returns the namespace prefix for this Tag, if any.
       * @return The namespace prefix.
       * @since 1.0
       */
      const std::string& prefix() const { return m_prefix; }

      /**
       * Returns the namespace prefix for the given namespace.
       * @return The namespace prefix for the given namespace.
       * @since 1.0
       */
      const std::string& prefix( const std::string& xmlns ) const;

      /* *
       * Adds an XML namespace declaration to the Tag. If @b def is false, a unique prefix will
       * be created, else the default namespace is set (no prefix).
       * @param xmlns The namespace value.
       * @param def If @b true, this sets the default namespace; if @b false, a unique namespace
       * prefix will be created (unless one already exists for the namespace) and used for
       * all subsequent references to the same namespace.
       * @since 1.0
       */
//       const std::string addXmlns( const std::string& xmlns, bool def );

      /**
       * Sets an XML namespace with a given prefix, or the default namespace if @c prefix
       * is empty.
       * @param xmlns The namespace value.
       * @param prefix An optional namespace prefix.
       * @return @b True if the input is valid UTF-8, @b false  otherwise. Invalid
       * input will be ignored.
       * @since 1.0
       */
      bool setXmlns( const std::string& xmlns, const std::string& prefix = EmptyString );

      /**
       * Returns the namespace for this element.
       * Namespace declarations in parent tags as well as prefixes will be taken into account.
       * @return The namespace for this element.
       * @since 1.0
       */
      const std::string& xmlns() const;

      /**
       * Returns the namespace for the given prefix, or the default namespace if
       * @c prefix is empty. Namespace declarations in parent tags will be taken into account.
       * Consider the following XML:
       * @code
       * &lt;foo:bar xmlns:foo='foobar'/&gt;
       * @endcode
       * &lt;bar/&gt; is in the @c foobar namespace, having a prefix of @b foo. A call to prefix()
       * will return 'foo'. A call to xmlns( "foo" ) or xmlns( prefix() ) will return 'foobar'.
       * A call to xmlns() will also return 'foobar' (it is a shortcut to
       * xmlns( prefix() ).
       * @param prefix The namespace prefix to look up, or an empty string to fetch the
       * default namespace.
       * @return The namespace for the given prefix, or the empty string if no such prefix exists.
       * The default namespace if an empty prefix is given.
       * @since 1.0
       */
      const std::string& xmlns( const std::string& prefix ) const;

      /**
       * Use this function to add a new attribute to the tag. The Tag will become the owner of the
       * Attribute and take care of deletion. If an Attribute with the same name already exists,
       * it will be replaced by the new one.
       * @param attr A pointer to the attribute to add.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       * @since 1.0
       * @note Do not use this function to set XML namespaces, use setXmlns() instead.
       */
      bool addAttribute( Attribute* attr );

      /**
       * Use this function to add a new attribute to the tag.
       * @param name The name of the attribute.
       * @param value The value of the attribute.
       * @note Do not use this function to set XML namespaces, use setXmlns() instead.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       */
      bool addAttribute( const std::string& name, const std::string& value );

      /**
       * Use this function to add a new attribute to the tag. The value is an @c int here.
       * @param name The name of the attribute.
       * @param value The value of the attribute.
       * @note Do not use this function to set XML namespaces, use setXmlns() instead.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       * @since 0.8
       */
      bool addAttribute( const std::string& name, int value );

      /**
       * Use this function to add a new attribute to the tag. The value is a @c long here.
       * @param name The name of the attribute.
       * @param value The value of the attribute.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       * @note Do not use this function to set XML namespaces, use setXmlns() instead.
       * @since 0.9
       */
      bool addAttribute( const std::string& name, long value );

      /**
       * Sets the given attributes. Any existing attributes are lost.
       * @param attributes The attributes to set.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       * @note Do not use this function to set XML namespaces, use setXmlns() instead.
       * @since 0.9
       */
      void setAttributes( const AttributeList& attributes );

      /**
       * Use this function to add a child node to the tag. The Tag will be owned by Tag.
       * @param child The node to be inserted.
       */
      void addChild( Tag* child );

      /**
       * Use this function to add a copy of the given element to the tag.
       * @param child The node to be inserted.
       * @since 0.9
       */
      void addChildCopy( const Tag* child );

      /**
       * Sets the XML character data for this Tag.
       * @param cdata The new cdata.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       */
      bool setCData( const std::string& cdata );

      /**
       * Adds the string to the existing XML character data for this Tag.
       * @param cdata The additional cdata.
       * @return @b True if the input is valid UTF-8, @b false otherwise. Invalid
       * input will be ignored.
       */
      bool addCData( const std::string& cdata );

      /**
       * Use this function to retrieve the name of an element.
       * @return The name of the tag.
       */
      const std::string& name() const { return m_name; }

      /**
       * Use this function to retrieve the XML character data of an element.
       * @return The cdata the element contains.
       */
      const std::string cdata() const;

      /**
       * Use this function to fetch a const list of attributes.
       * @return A constant reference to the list of attributes.
       */
      const AttributeList& attributes() const;

      /**
       * Use this function to fetch a const list of child elements.
       * @return A constant reference to the list of child elements.
       */
      const TagList& children() const;

      /**
       * This function can be used to retrieve the value of a Tag's attribute.
       * @param name The name of the attribute to look for.
       * @return The value of the attribute if found, an empty string otherwise.
       */
      const std::string& findAttribute( const std::string& name ) const;

      /**
       * Checks whether the tag has a attribute with given name and optional value.
       * @param name The name of the attribute to check for.
       * @param value The value of the attribute to check for.
       * @return Whether the attribute exists (optionally with the given value).
       */
      bool hasAttribute( const std::string& name, const std::string& value = EmptyString ) const;

      /**
       * This function finds and returns the @b first element within the child elements of the current tag
       * that has a matching tag name.
       * @param name The name of the element to search for.
       * @return The found Tag, or 0.
       */
      Tag* findChild( const std::string& name ) const;

      /**
       * This function finds and returns the @b first element within the child elements of the current tag,
       * that has a certain name, and a certain attribute with a certain value.
       * @param name The name of the element to search for.
       * @param attr The name of the attribute of the child element.
       * @param value The value of the attribute of the child element.
       * @return The found Tag, or 0.
       */
      Tag* findChild( const std::string& name, const std::string& attr,
                      const std::string& value = EmptyString ) const;

      /**
       * This function checks whether the Tag has a child element with a given name, and optionally
       * this child element is checked for having a given attribute with an optional value.
       * @param name The name of the child element.
       * @param attr The name of the attribute of the child element.
       * @param value The value of the attribute of the child element.
       * @return @b True if the given child element exists, @b false otherwise.
       */
      bool hasChild( const std::string& name, const std::string& attr = EmptyString,
                     const std::string& value = EmptyString ) const;

      /**
       * This function checks whether the Tag has a child element which posesses a given attribute
       * with an optional value. The name of the child element does not matter.
       * @param attr The name of the attribute of the child element.
       * @param value The value of the attribute of the child element.
       * @return The child if found, 0 otherwise.
       */
      Tag* findChildWithAttrib( const std::string& attr, const std::string& value = EmptyString ) const;

      /**
       * This function checks whether the Tag has a child element which posesses a given attribute
       * with an optional value. The name of the child element does not matter.
       * @param attr The name of the attribute of the child element.
       * @param value The value of the attribute of the child element.
       * @return @b True if any such child element exists, @b false otherwise.
       */
      inline bool hasChildWithAttrib( const std::string& attr,
                                      const std::string& value = EmptyString ) const
        { return findChildWithAttrib( attr, value ) ? true : false; }

      /**
       * Returns a list of child tags of the current tag with the given name.
       * @param name The name of the tags to look for.
       * @param xmlns An optional namespace to check for.
       * @return A list of tags with the given name.
       * @note The tags are still linked to the current Tag and should not be deleted.
       * @since 0.9
       */
      TagList findChildren( const std::string& name, const std::string& xmlns = EmptyString ) const;

      /**
       * Removes and deletes all child tags that have the given name and are, optionally,
       * within the given namespace.
       * @param name The name of the tag(s) to remove from the list of child tags.
       * @param xmlns An optional namespace to check for.
       */
      void removeChild( const std::string& name, const std::string& xmlns = EmptyString );

      /**
       * Removes the given Tag from the list of child Tags.
       * @param tag The Tag to remove from the list of child Tags.
       * @note The Tag @p tag is not deleted.
       */
      void removeChild( Tag* tag );

      /**
       * Removes the attribute with the given name and optional value from this Tag.
       * @param attr The attribute's name.
       * @param value The attribute's optional value.
       * @param xmlns An optional namespace to check for.
       */
      void removeAttribute( const std::string& attr, const std::string& value = EmptyString,
                            const std::string& xmlns = EmptyString );

      /**
       * This function checks whether a child element with given name exists and has
       * XML character data that equals the given cdata string.
       * @param name The name of the child element.
       * @param cdata The character data that has to exist in the child element.
       * @return @b True if a child element with given cdata exists, @b false otherwise.
       */
      bool hasChildWithCData( const std::string& name, const std::string& cdata ) const;

      /**
       * Returns the tag's parent Tag.
       * @return The Tag above the current Tag. May be @b 0.
       */
      Tag* parent() const { return m_parent; }

      /**
       * This function creates a deep copy of this Tag.
       * @return An independent copy of the Tag.
       * @since 0.7
       */
      Tag* clone() const;

      /**
       * Evaluates the given XPath expression and returns the result Tag's character data, if any.
       * If more than one Tag match, only the first one's character data is returned.
       * @note Currently, XPath support is somewhat limited. However, it should be useable
       * for basic expressions. For now, see src/tests/xpath/xpath_test.cpp for supported
       * expressions.
       * @param expression An XPath expression to evaluate.
       * @return A matched Tag's character data, or the empty string.
       * @since 1.0
       */
      const std::string findCData( const std::string& expression ) const;

      /**
       * Evaluates the given XPath expression and returns the result Tag. If more than one
       * Tag match, only the first one is returned.
       * @note Currently, XPath support is somewhat limited. However, it should be useable
       * for basic expressions. For now, see src/tests/xpath/xpath_test.cpp for supported
       * expressions.
       * @param expression An XPath expression to evaluate.
       * @return A matched Tag, or 0.
       * @since 0.9
       */
      const Tag* findTag( const std::string& expression ) const;

      /**
       * Evaluates the given XPath expression and returns the matched Tags.
       * @note Currently, XPath support is somewhat limited. However, it should be useable
       * for basic expressions. For now, see src/tests/xpath/xpath_test.cpp for supported
       * expressions.
       * @param expression An XPath expression to evaluate.
       * @return A list of matched Tags, or an empty TagList.
       * @since 0.9
       */
      ConstTagList findTagList( const std::string& expression ) const;

      /**
       * Checks two Tags for equality. Order of attributes and child tags does matter.
       * @param right The Tag to check against the current Tag.
       * @since 0.9
       */
      bool operator==( const Tag &right ) const;

      /**
       * Checks two Tags for inequality. Order of attributes and child tags does matter.
       * @param right The Tag to check against the current Tag.
       * @since 0.9
       */
      bool operator!=( const Tag &right ) const { return !( *this == right ); }

      /**
       * Returns @b true if the Tag is valid, @b false otherwise.
       */
      operator bool() const { return !m_name.empty(); }

    private:
      /**
       * Creates a new Tag by stealing the original Tag's body (elements, attributes). The
       * original Tag is pretty much useless afterwards.
       * @param tag The Tag to rip off.
       */
      Tag( Tag* tag );

      /**
       * XPath error conditions.
       */
      enum XPathError
      {
        XPNoError,                  /**< No error occured. */
        XPExpectedLeftOperand,      /**< Operator expected a left-hand operand. */
        XPUnexpectedToken
      };

      enum NodeType
      {
        TypeTag,                    /**< The Node is a Tag. */
        TypeString                  /**< The Node is a std::string. */
      };

      struct Node
      {
        Node( NodeType _type, Tag* _tag ) : type( _type ), tag( _tag ) {}
        Node( NodeType _type, std::string* _str ) : type( _type ), str( _str ) {}
        ~Node() {}

        NodeType type;
        union
        {
          Tag* tag;
          std::string* str;
        };
      };

      typedef std::list<Node*> NodeList;

      Tag* m_parent;
      TagList* m_children;
      StringPList* m_cdata;
      AttributeList* m_attribs;
      NodeList* m_nodes;
      std::string m_name;
      std::string m_xmlns;
      StringMap* m_xmlnss;
      std::string m_prefix;

      enum TokenType
      {
        XTNone,
        XTLeftParenthesis,
        XTRightParenthesis,
        XTNodeSet,
        XTInteger,
        XTElement,
        XTLeftBracket,
        XTRightBracket,
        XTFunction,
        XTAsterisk,
        XTAttribute,
        XTLiteralInside,
        XTLiteral,
        XTDot,
        XTDoubleDot,
        XTOperatorOr,
        XTOperatorAnd,
        XTOperatorEq,
        XTOperatorNe,
        XTOperatorGt,
        XTOperatorLt,
        XTOperatorLtEq,
        XTOperatorGtEq,
        XTOperatorPlus,
        XTOperatorMinus,
        XTOperatorMul,
        XTOperatorDiv,
        XTOperatorMod,
        XTUnion,
        XTSlash,
        XTDoubleSlash
      };

      /**
       * Sets a list of namespaces.
       * @param xmlnss The list of namespaces.
       * @since 1.0
       */
      void setXmlns( StringMap* xmlns )
        { delete m_xmlnss; m_xmlnss = xmlns; }

      Tag* parse( const std::string& expression, unsigned& len, TokenType border = XTNone ) const;

      void closePreviousToken( Tag**, Tag**, TokenType&, std::string& ) const;
      void addToken( Tag **root, Tag **current, TokenType type, const std::string& token ) const;
      void addOperator( Tag **root, Tag **current, Tag* arg, TokenType type,
                        const std::string& token ) const;
      bool addPredicate( Tag **root, Tag **current, Tag* token ) const;

      TagList findChildren( const TagList& list, const std::string& name,
                            const std::string& xmlns = EmptyString ) const;
      ConstTagList evaluateTagList( Tag* token ) const;
      ConstTagList evaluateUnion( Tag* token ) const;
      ConstTagList allDescendants() const;

      static TokenType getType( const std::string& c );

      static bool isWhitespace( const char c );
      bool isNumber() const;

      bool evaluateBoolean( Tag* token ) const;
      bool evaluatePredicate( Tag* token ) const { return evaluateBoolean( token ); }
      bool evaluateEquals( Tag* token ) const;

      static void add( ConstTagList& one, const ConstTagList& two );
  };

}

#endif // TAG_H__
