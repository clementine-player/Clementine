/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef PUBSUBITEM_H__
#define PUBSUBITEM_H__

#include "gloox.h"

#include <string>

namespace gloox
{

  class Tag;

  namespace PubSub
  {

    /**
     * @brief Abstracts a PubSub Item (XEP-0060).
     *
     * XEP Version: 1.12
     *
     * @author Jakob Schroeter <js@camaya.net>
     * @since 1.0
     */
    class GLOOX_API Item
    {
      public:
        /**
        * Constructs a new empty Item.
        */
        Item();

        /**
        * Constructs a new Item from the given Tag.
        * @param tag The Tag to parse.
        */
        Item( const Tag* tag );

        /**
         * Copy constructor.
         * @param item The Item to be copied.
         */
        Item( const Item& item );

        /**
        * Destructor.
        */
        ~Item();

        /**
         * Returns the Item's payload.
         * @return The layload.
         */
        const Tag* payload() const { return m_payload; }

        /**
         * Returns the item ID.
         * @return The item ID.
         */
        const std::string& id() const { return m_id; }

        /**
         * Creates and returns a Tag representation of the Item.
         * @return An XML representation of the Item.
         */
        Tag* tag() const;

      private:
        Tag* m_payload;
        std::string m_id;

    };

  }

}

#endif // PUBSUBITEM_H__
