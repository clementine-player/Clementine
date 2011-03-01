/*
 * Copyright (c) 2009 by Jakob Schroeter <js@camaya.net>
 * This file is part of the gloox library. http://camaya.net/gloox
 *
 * This software is distributed under a license. The full license
 * agreement can be found in the file LICENSE in this distribution.
 * This software may not be copied, modified, sold or distributed
 * other than expressed in the named license agreement.
 *
 * This software is distributed without any warranty.
 */


#ifndef COMPRESSIONDEFAULT_H__
#define COMPRESSIONDEFAULT_H__

#include "compressionbase.h"

namespace gloox
{

  class CompressionDataHandler;

  /**
   * @brief This is an abstraction of the various Compression implementations.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API CompressionDefault : public CompressionBase
  {
    public:

      /**
       * Supported ctypes.
       */
      enum Method
      {
        MethodZlib   =  1,          /**< Zlib compression. */
        MethodLZW    =  2           /**< LZW compression. */
      };

      /**
       * Constructs a new compression wrapper.
       * @param cdh The CompressionDataHandler to handle de/compressed data.
       * @param method The desired compression method.
       */
      CompressionDefault( CompressionDataHandler* cdh, Method method = MethodZlib );

      /**
       * Virtual Destructor.
       */
      virtual ~CompressionDefault();

      /**
       * Returns an int holding the available compression types, ORed.
       * @return An int holding the available compression types, ORed.
       */
      static int types();

      // reimplemented from CompressionBase
      virtual bool init();

      // reimplemented from CompressionBase
      virtual void compress( const std::string& data );

      // reimplemented from CompressionBase
      virtual void decompress( const std::string& data );

      // reimplemented from CompressionBase
      virtual void cleanup();

    private:
      CompressionBase* m_impl;

  };

}

#endif // COMPRESSIONDEFAULT_H__
