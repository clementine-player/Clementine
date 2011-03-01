/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef STANZAEXTENSIONFACTORY_H__
#define STANZAEXTENSIONFACTORY_H__

#include <list>

namespace gloox
{

  class Tag;
  class Stanza;
  class StanzaExtension;

  /**
   * @brief A Factory that creates StanzaExtensions from Tags.
   *
   * To supply a custom StanzaExtension, reimplement StanzaExtension's
   * virtuals and pass an instance to registerExtension().
   *
   * You should not need to use this class directly. Use ClientBase::registerStanzaExtension()
   * instead. See StanzaExtension for more information about adding protocol implementations
   * to gloox.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class StanzaExtensionFactory
  {

    friend class ClientBase;

    public:
      /**
       * Use this function to inform StanzaExtensionFactory about available StanzaExtensions.
       * By default, StanzaExtensionFactory does not know about any extensions.
       * gloox-built-in extensions will usually be registered by their respective protocol
       * implementations unless otherwise noted in the extension's API docs.
       * @param ext An extension to register.
       * @note The supplied StanzaExtension will be deleted in StanzaExtensionFactory's destructor.
       * @note Only one instance per extension type can be registered. In case an extension is
       * registered that is of the same type as an already registered extension, the new extension
       * will replace the previously registered one.
       */
      void registerExtension( StanzaExtension* ext );

      /**
       * Removes the given extension type.
       * @param ext The extension type.
       * @return @b True if the given type was found (and removed), @b false otherwise.
       */
      bool removeExtension( int ext );

      /**
       * Creates a new StanzaExtensionFactory.
       */
      StanzaExtensionFactory();

      /**
       * Non-virtual destructor.
       */
      ~StanzaExtensionFactory();

      /**
       * This function creates StanzaExtensions from the given Tag and attaches them to the given Stanza.
       * @param stanza The Stanza to attach the extensions to.
       * @param tag The Tag to parse and create the StanzaExtension from.
       */
      void addExtensions( Stanza& stanza, Tag* tag );

    private:
      typedef std::list<StanzaExtension*> SEList;
      SEList m_extensions;

  };

}

#endif // STANZAEXTENSIONFACTORY_H__
