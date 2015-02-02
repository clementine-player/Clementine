/*
   Copyright 2009 Last.fm Ltd. 
   Copyright 2009 John Stamp <jstamp@users.sourceforge.net>

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LASTFM_FINGERPRINTABLE_SOURCE_H
#define LASTFM_FINGERPRINTABLE_SOURCE_H

#include "global.h"
#include <QString>

namespace lastfm
{
    class LASTFM_FINGERPRINT_DLLEXPORT FingerprintableSource
    {
    public:
        virtual ~FingerprintableSource() {}

        /** do all initialisation here and throw if there is problems */
        virtual void init( const QString& path ) = 0;

        virtual void getInfo( int& lengthSecs, int& samplerate, int& bitrate, int& nchannels ) = 0;

        /** put a chunk of PCM data in pBuffer, don't exceed size, return the
          * number of bytes you put in the buffer */
        virtual int updateBuffer( signed short* buffer, size_t bufferSize ) = 0;

        virtual void skip( const int mSecs ) = 0;
        virtual void skipSilence( double silenceThreshold = 0.0001 ) = 0;

        virtual bool eof() const = 0;
    };
}

#endif
