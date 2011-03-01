/*
  Copyright (c) 2008-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef SOFTWAREVERSION_H__
#define SOFTWAREVERSION_H__


#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an implementation of XEP-0092 as a StanzaExtension.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API SoftwareVersion : public StanzaExtension
  {

    public:
    /**
     * Constructs a new object with the given resource string.
     * @param name The software's name.
     * @param version The software's version.
     * @param os The software's operating system.
     */
      SoftwareVersion( const std::string& name, const std::string& version, const std::string& os );

      /**
       * Constructs a new object from the given Tag.
       * @param tag The Tag to parse.
       */
      SoftwareVersion( const Tag* tag = 0 );

      /**
       * Virtual Destructor.
       */
      virtual ~SoftwareVersion();

      /**
       * Returns the application's name.
       * @return The application's name.
       */
      const std::string& name() const { return m_name; }

      /**
       * Returns the application's version.
       * @return The application's version.
       */
      const std::string& version() const { return m_version; }

      /**
       * Returns the application's Operating System.
       * @return The application's OS.
       */
      const std::string& os() const { return m_os; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new SoftwareVersion( tag );
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new SoftwareVersion( *this );
      }

    private:
      std::string m_name;
      std::string m_version;
      std::string m_os;
  };

}

#endif// SOFTWAREVERSION_H__
