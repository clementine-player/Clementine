/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef FEATURENEG_H__
#define FEATURENEG_H__

#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class DataForm;
  class Tag;

  /**
   * @brief An abstraction of Feature Negotiation (XEP-0020), implemented
   * as a StanzaExtension.
   *
   * XEP Version: 1.5
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API FeatureNeg : public StanzaExtension
  {
    public:
      /**
       * Creates a new wrapper object using the given DataForm.
       * @param form The DataForm to embed. The FeatureNeg object will own the DataForm.
       */
      FeatureNeg( DataForm* form );

      /**
       * Creates a new wrapper object from the given Tag.
       * @param tag The Tag to parse.
       */
      FeatureNeg( const Tag* tag = 0 );

      /**
       * Virtual destructor.
       */
      virtual ~FeatureNeg();

      /**
       * Returns the wrapped DataForm.
       * @return The wrapped DataForm. May be 0.
       */
      const DataForm* form() const { return m_form; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new FeatureNeg( tag );
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new FeatureNeg( m_form );
      }

    private:
      DataForm* m_form;

  };

}

#endif // FEATURENEG_H__
