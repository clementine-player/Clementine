/***************************************************************************
    copyright            : (C) 2016 by Damien Plisson, Audirvana
    email                : damien78@audirvana.com
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

#ifndef TAGLIB_DSDIFFPROPERTIES_H
#define TAGLIB_DSDIFFPROPERTIES_H

#include "audioproperties.h"

namespace TagLib {

  namespace DSDIFF {

    class File;

    //! An implementation of audio property reading for DSDIFF

    /*!
     * This reads the data from an DSDIFF stream found in the AudioProperties
     * API.
     */

    class TAGLIB_EXPORT Properties : public AudioProperties
    {
    public:
      /*!
       * Create an instance of DSDIFF::Properties with the data read from the
       * ByteVector \a data.
       */
      Properties(const unsigned int sampleRate, const unsigned short channels,
                 const unsigned long long samplesCount, const int bitrate,
                 ReadStyle style);

      /*!
       * Destroys this DSDIFF::Properties instance.
       */
      virtual ~Properties();

      // Reimplementations.

      virtual int length() const;
      virtual int lengthInSeconds() const;
      virtual int lengthInMilliseconds() const;
      virtual int bitrate() const;
      virtual int sampleRate() const;
      virtual int channels() const;

      int bitsPerSample() const;
      long long sampleCount() const;

    private:
      Properties(const Properties &);
      Properties &operator=(const Properties &);

      class PropertiesPrivate;
      PropertiesPrivate *d;
    };
  }
}

#endif

