/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef SHA_H__
#define SHA_H__

#include "macros.h"

#include <string>

namespace gloox
{

  /**
   * @brief An implementation of SHA1.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SHA
  {

    public:
      /**
       * Constructs a new SHA object.
       */
      SHA();

      /**
       * Virtual Destructor.
       */
      virtual ~SHA();

      /**
       * Resets the internal state.
       */
      void reset();

      /**
       * Finalizes the hash computation.
       */
      void finalize();

      /**
       * Returns the message digest in hex notation. Finalizes the hash if finalize()
       * has not been called before.
       * @return The message digest.
       */
      const std::string hex();

      /**
       * Returns the raw binary message digest. Finalizes the hash if finalize()
       * has not been called before.
       * @return The message raw binary digest.
       */
      const std::string binary();

      /**
       * Provide input to SHA1.
       * @param data The data to compute the digest of.
       * @param length The size of the data in bytes.
       */
      void feed( const unsigned char* data, unsigned length );

      /**
       * Provide input to SHA1.
       * @param data The data to compute the digest of.
       */
      void feed( const std::string& data );

    private:
      void process();
      void pad();
      inline unsigned shift( int bits, unsigned word );
      void init();

      unsigned H[5];
      unsigned Length_Low;
      unsigned Length_High;
      unsigned char Message_Block[64];
      int Message_Block_Index;
      bool m_finished;
      bool m_corrupted;

  };

}

#endif // SHA_H__
