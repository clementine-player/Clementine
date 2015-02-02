/*
   Copyright 2011 Last.fm Ltd.
      - Primarily authored by Michael Coffey

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

#ifndef TASTEOMETER_H
#define TASTEOMETER_H

#include "global.h"

class QNetworkReply;

namespace lastfm
{
    class User;

    class LASTFM_DLLEXPORT Tasteometer
    {
    public:
        Tasteometer();
        ~Tasteometer();

    public:
        static QNetworkReply* compare( const User& left, const User& right );
    };
}

#endif // TASTEOMETER_H
