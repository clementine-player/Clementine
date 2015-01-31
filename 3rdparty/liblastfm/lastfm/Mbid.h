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
#ifndef LASTFM_MBID_H
#define LASTFM_MBID_H

#include "global.h"

namespace lastfm
{
    class LASTFM_DLLEXPORT Mbid
    {
    public:
        explicit Mbid( const QString& p = "" );
        Mbid( const Mbid& that );
        ~Mbid();

        bool isNull() const;
        operator QString() const;
        Mbid& operator=( const Mbid& that );

        /** if this is not an mp3 file you will be wasting time, as it won't work
          * but we will do what you say anyway because you are the boss */ 
        static Mbid fromLocalFile( const QString& path );
    private:
        class MbidPrivate * const d;
    };
}

#endif
