/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef COMPRESSIONDATAHANDLER_H__
#define COMPRESSIONDATAHANDLER_H__

#include "macros.h"

#include <string>

namespace gloox
{

  /**
   * @brief An abstract base class used to receive de/compressed data from a
   * CompressionBase-derived object.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API CompressionDataHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~CompressionDataHandler() {}

      /**
       * This function is called when compression is finished.
       * @param data The compressed data.
       */
      virtual void handleCompressedData( const std::string& data ) = 0;

      /**
       * This function is called when decompression is finished.
       * @param data The decompressed data.
       */
      virtual void handleDecompressedData( const std::string& data ) = 0;

  };

}

#endif // COMPRESSIONDATAHANDLER_H__
