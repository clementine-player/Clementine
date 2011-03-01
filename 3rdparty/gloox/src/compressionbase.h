/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef COMPRESSIONBASE_H__
#define COMPRESSIONBASE_H__

#include "gloox.h"
#include "compressiondatahandler.h"

#include <string>

namespace gloox
{

  /**
   * @brief This is an abstract base class for stream compression implementations.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API CompressionBase
  {
    public:
      /**
       * Contructor.
       * @param cdh A CompressionDataHandler-derived object that will be notified
       * about finished de/compression.
       */
      CompressionBase( CompressionDataHandler* cdh ) : m_handler( cdh ), m_valid( false ) {}

      /**
       * Virtual Destructor.
       */
      virtual ~CompressionBase() {}

      /**
       * This function initializes the compression module.
       * it is mandatory to be called.
       * @return @b True if the module was initialized successfully, false otherwise.
       */
      virtual bool init() = 0;

      /**
       * Compresses the given chunk of data.
       * @param data The original (uncompressed) data.
       */
      virtual void compress( const std::string& data ) = 0;

      /**
       * Decompresses the given chunk of data.
       * @param data The compressed data.
       */
      virtual void decompress( const std::string& data ) = 0;

      /**
       * Performs internal cleanup.
       * @since 1.0
       */
      virtual void cleanup() = 0;

    protected:
      /** A handler for compressed/uncompressed data. */
      CompressionDataHandler* m_handler;

      /** Whether the compression module can be used. */
      bool m_valid;

  };

}

#endif // COMPRESSIONBASE_H__
