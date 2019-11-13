/***************************************************************************
    copyright            : (C) 2013 by Stephen F. Booth
    email                : me@sbooth.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_DSFPROPERTIES_H
#define TAGLIB_DSFPROPERTIES_H

#include "audioproperties.h"

namespace TagLib {

  namespace DSF {

    class File;

    //! An implementation of audio property reading for DSF

    /*!
     * This reads the data from a DSF stream found in the AudioProperties
     * API.
     */

    class TAGLIB_EXPORT Properties : public TagLib::AudioProperties
    {
    public:
      /*!
       * Create an instance of DSF::AudioProperties with the data read from the
       * ByteVector \a data.
       */
      Properties(const ByteVector &data, ReadStyle style);

      /*!
       * Destroys this DSF::AudioProperties instance.
        */
      virtual ~Properties();

      // Reimplementations.

      virtual int length() const;
      virtual int lengthInSeconds() const;
      virtual int lengthInMilliseconds() const;
      virtual int bitrate() const;
      virtual int sampleRate() const;
      virtual int channels() const;

      int formatVersion() const;
      int formatID() const;

      /*!
       * Channel type values: 1 = mono, 2 = stereo, 3 = 3 channels, 
       * 4 = quad, 5 = 4 channels, 6 = 5 channels, 7 = 5.1 channels
       */
      int channelType() const;
      int bitsPerSample() const;
      long long sampleCount() const;
      int blockSizePerChannel() const;

    private:
      Properties(const Properties &);
      Properties &operator=(const Properties &);

      void read(const ByteVector &data);

      class PropertiesPrivate;
      PropertiesPrivate *d;
    };
  }
}

#endif

