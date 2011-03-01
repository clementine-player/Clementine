/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef COMPRESSIONZLIB_H__
#define COMPRESSIONZLIB_H__

#include "compressionbase.h"
#include "mutex.h"

#include "config.h"

#ifdef HAVE_ZLIB

#include <zlib.h>

#include <string>

namespace gloox
{
  /**
   * An implementation of CompressionBase using zlib.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API CompressionZlib : public CompressionBase
  {
    public:
      /**
       * Contructor.
       * @param cdh The CompressionDataHandler to receive de/compressed data.
       */
      CompressionZlib( CompressionDataHandler* cdh );

      /**
       * Virtual Destructor.
       */
      virtual ~CompressionZlib();

      // reimplemented from CompressionBase
      virtual bool init();

      // reimplemented from CompressionBase
      virtual void compress( const std::string& data );

      // reimplemented from CompressionBase
      virtual void decompress( const std::string& data );

      // reimplemented from CompressionBase
      virtual void cleanup();

    private:
      z_stream m_zinflate;
      z_stream m_zdeflate;

      util::Mutex m_compressMutex;

  };

}

#endif // HAVE_ZLIB

#endif // COMPRESSIONZLIB_H__
