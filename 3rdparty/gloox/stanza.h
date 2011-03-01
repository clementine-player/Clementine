/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef STANZA_H__
#define STANZA_H__

#include "gloox.h"
#include "tag.h"
#include "jid.h"
#include "stanzaextension.h"

namespace gloox
{

  class Error;

  /**
   * @brief This is the base class for XMPP stanza abstractions.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.4
   */
  class GLOOX_API Stanza
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~Stanza();

      /**
       * Sets the 'from' address of the Stanza. This useful for @link gloox::Component Components @endlink.
       * @param from The from address.
       */
      void setFrom( const JID& from ) { m_from = from; }

      /**
       * Returns the JID the stanza comes from.
       * @return The origin of the stanza.
       */
      const JID& from() const { return m_from; }

      /**
       * Returns the receiver of the stanza.
       * @return The stanza's destination.
       */
      const JID& to() const { return m_to; }

      /**
       * Returns the id of the stanza, if set.
       * @return The ID of the stanza.
       */
      const std::string& id() const { return m_id; }

      /**
       * A convenience function that returns the stanza error condition, if any.
       * @return The stanza error condition, may be 0.
       */
      const Error* error() const;

      /**
       * Retrieves the value of the xml:lang attribute of this stanza.
       * Default is 'en'.
       * @return The stanza's default language.
       */
      const std::string& xmlLang() const { return m_xmllang; }

      /**
       * Use this function to add a StanzaExtension to this Stanza.
       * @param se The StanzaExtension to add.
       * @note The Stanza will become the owner of the StanzaExtension and
       * will take care of deletion.
       * @since 0.9
       */
      void addExtension( const StanzaExtension* se );

      /**
       * Finds a StanzaExtension of a particular type.
       * @param type StanzaExtensionType to search for.
       * @return A pointer to the StanzaExtension, or 0 if none was found.
       */
      const StanzaExtension* findExtension( int type ) const;

      /**
       * Finds a StanzaExtension of a particular type.
       * Example:
       * @code
       * const MyExtension* c = presence.findExtension<MyExtension>( ExtMyExt );
       * @endcode
       * @param type The extension type to look for.
       * @return The static_cast' type, or 0 if none was found.
       */
      template< class T >
      inline const T* findExtension( int type ) const
      {
        return static_cast<const T*>( findExtension( type ) );
      }

      /**
       * Returns the list of the Stanza's extensions.
       * @return The list of the Stanza's extensions.
       */
      const StanzaExtensionList& extensions() const { return m_extensionList; }

      /**
       * Removes (deletes) all the stanza's extensions.
       */
      void removeExtensions();

      /**
       * Creates a Tag representation of the Stanza. The Tag is completely
       * independent of the Stanza and will not be updated when the Stanza
       * is modified.
       * @return A pointer to a Tag representation. It is the job of the
       * caller to delete the Tag.
       */
      virtual Tag* tag() const = 0;

    protected:
      /**
       * Creates a new Stanza, taking from and to addresses from the given Tag.
       * @param tag The Tag to create the Stanza from.
       * @since 1.0
       */
      Stanza( Tag* tag );

      /**
       * Creates a new Stanza object and initializes the receiver's JID.
       * @param to The receipient of the Stanza.
       * @since 1.0
       */
      Stanza( const JID& to );

      StanzaExtensionList m_extensionList;
      std::string m_id;
      std::string m_xmllang;
      JID m_from;
      JID m_to;

      static const std::string& findLang( const StringMap* map,
                                          const std::string& defaultData,
                                          const std::string& lang );

      static void setLang( StringMap** map,
                           std::string& defaultLang,
                           const Tag* tag );

      static void setLang( StringMap** map,
                           std::string& defaultLang,
                           const std::string& data,
                           const std::string& xmllang );

      static void getLangs( const StringMap* map,
                            const std::string& defaultData,
                            const std::string& name, Tag* tag );

    private:
      Stanza( const Stanza& );

  };

}

#endif // STANZA_H__
