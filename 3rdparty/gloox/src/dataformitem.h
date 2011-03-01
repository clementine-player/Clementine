/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DATAFORMITEM_H__
#define DATAFORMITEM_H__

#include "dataformfieldcontainer.h"

namespace gloox
{

  /**
   * @brief An abstraction of an &lt;item&gt; element in a XEP-0004 Data Form of type result.
   *
   * There are some constraints regarding usage of this element you should be aware of. Check XEP-0004
   * section 3.4. This class does not enforce correct usage at this point.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API DataFormItem : public DataFormFieldContainer
  {
    public:
      /**
       * Creates an empty 'item' element you can add fields to.
       */
      DataFormItem();

      /**
       * Creates a 'item' element and fills it with the 'field' elements contained in the given Tag.
       * The Tag's root element must be a 'item' element. Its child element should be 'field' elements.
       * @param tag The tag to read the 'field' elements from.
       * @since 0.8.5
       */
      DataFormItem( const Tag* tag );

      /**
       * Virtual destructor.
       */
      virtual ~DataFormItem();

      /**
       * Creates and returns a Tag representation of the current object.
       * @return A Tag representation of the current object.
       */
      virtual Tag* tag() const;

  };

}

#endif // DATAFORMITEM_H__
