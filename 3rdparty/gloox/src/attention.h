/*
  Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ATTENTION_H__
#define ATTENTION_H__


#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an implementation of XEP-0224 as a StanzaExtension.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Attention : public StanzaExtension
  {

    public:
      /**
       * Constructs a new object from the given Tag.
       */
      Attention();

      /**
       * Virtual Destructor.
       */
      virtual ~Attention();

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* /*tag*/ ) const
      {
        return new Attention();
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new Attention();
      }

  };

}

#endif// ATTENTION_H__
