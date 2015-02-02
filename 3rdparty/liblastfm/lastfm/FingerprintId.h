/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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
#ifndef LASTFM_FINGERPRINT_ID_H
#define LASTFM_FINGERPRINT_ID_H

#include "Track.h"
 
namespace lastfm
{
    class LASTFM_DLLEXPORT FingerprintId
    {
    public:
        FingerprintId();
        FingerprintId( uint i );
        FingerprintId( const FingerprintId& other );

        ~FingerprintId();

        bool isNull() const;

        /** we query Last.fm for suggested metadata, how awesome is that? 
          * @returns null if isNull() */
        QNetworkReply* getSuggestions() const;
        static QMap<float,Track> getSuggestions( QNetworkReply* );

        /** -1 if you need to generate it */
        operator int() const;
        /** isEmpty() if you need to generate it */
        operator QString() const;
        FingerprintId& operator=( const FingerprintId& other );

    private:
        class FingerprintIdPrivate * const d;
    };
}


LASTFM_DLLEXPORT QDebug operator<<( QDebug d, lastfm::FingerprintId id);

#endif
