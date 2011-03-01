/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef PRIVATEXMLHANDLER_H__
#define PRIVATEXMLHANDLER_H__

#include "macros.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief A virtual interface which can be reimplemented to store and receive private XML data.
   *
   * Derived classes can be registered as PrivateXMLHandlers with the PrivateXML object.
   * Upon an incoming PrivateXML packet @ref handlePrivateXML() will be called.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API PrivateXMLHandler
  {
    public:
      /**
       * Describes the possible results of a 'store' or 'request' operation.
       */
      enum PrivateXMLResult
      {
        PxmlStoreOk,                /**< Storing was successful. */
        PxmlStoreError,             /**< An error occurred while storing data in Private XML. */
        PxmlRequestError            /**< An error occurred while requesting Private XML. */
      };

      /**
       * Virtual Destructor.
       */
      virtual ~PrivateXMLHandler() {}

      /**
       * Reimplement this function to receive the private XML that was requested earlier using
       * @c PrivateXML::requestXML().
       * @param xml The private xml, i.e. the first child of the &lt;query&gt; tag.
       * May be 0. You should not delete the object.
       */
      virtual void handlePrivateXML( const Tag* xml ) = 0;

      /**
       * This function is called to notify about the result of a 'store' or 'request' operation
       * (successful requests are announced by means of handlePrivateXML()).
       * @param uid The ID of the query.
       * @param pxResult The result of the operation.
       * @since 0.7
       */
      virtual void handlePrivateXMLResult( const std::string& uid, PrivateXMLResult pxResult ) = 0;
  };

}

#endif // PRIVATEXMLHANDLER_H__
