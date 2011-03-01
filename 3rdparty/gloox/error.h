/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef ERROR_H__
#define ERROR_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>
#include <map>

namespace gloox
{

  class Tag;

  /**
   * @brief A stanza error abstraction implemented as a StanzaExtension.
   *
   * @author Vincent Thomasset
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Error : public StanzaExtension
  {
    public:

//       Error()
//         : StanzaExtension( ExtError ), m_type( StanzaErrorTypeUndefined ),
//           m_error( StanzaErrorUndefined ), m_appError( 0 )
//       {}

      /**
       * Creates a new Error object from the given Tag.
       * @param tag The Tag to parse.
       */
      Error( const Tag* tag = 0 );

      /**
       * Creates a new Error object.
       * @param type The error type.
       * @param error The actual stanza error.
       * @param appError An optional application-specific error.
       */
      Error( StanzaErrorType type, StanzaError error, Tag* appError = 0 )
        : StanzaExtension( ExtError ), m_type( type ),
          m_error( error ), m_appError( appError )
      {}

      /**
       * Virtual destructor.
       */
      virtual ~Error();

      /**
       * Returns the error type.
       * @return The error type.
       */
      StanzaErrorType type() const { return m_type; }

      /**
       * Return the stanza error.
       * @return The actual error.
       */
      StanzaError error() const { return m_error; }

      /**
       * This function can be used to retrieve the application-specific error
       * condition of a stanza error.
       * @return The application-specific error element of a stanza error.
       * 0 if no respective element was found or no error occured.
       */
      const Tag* appError() const { return m_appError; }

      /**
       * Returns the text of a error stanza for the given language if available.
       * If the requested language is not available, the default text (without
       * a xml:lang attribute) will be returned.
       * @param lang The language identifier for the desired language. It must
       * conform to section 2.12 of the XML specification and RFC 3066. If
       * empty, the default text will be returned, if any.
       * @return The text of an error stanza.
       */
      const std::string& text( const std::string& lang = EmptyString ) const;

      /**
       * Sets the text of a error stanza for the given language.
       * @param text The error text to set.
       * @param lang The language identifier for the desired language. It must
       * conform to section 2.12 of the XML specification and RFC 3066. If
       * empty, the default text will be set.
       */
      void setText( const std::string& text, const std::string& lang = EmptyString )
      {
        m_text[lang] = text;
      }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new Error( tag );
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new Error( *this );
      }

    private:
      Error( const Error& error );

      void setValues( const Tag* tag );

      StanzaErrorType m_type;
      StanzaError m_error;
      Tag* m_appError;
      StringMap m_text;
  };

}

#endif /* ERROR_H__ */
