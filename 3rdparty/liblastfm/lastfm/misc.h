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
#ifndef LASTFM_MISC_H
#define LASTFM_MISC_H

#include "global.h"
#include <QDir>

#ifdef Q_OS_MAC
typedef const struct __CFString* CFStringRef;
#endif

namespace lastfm
{
    namespace dir
    {
    #ifdef Q_OS_WIN
        LASTFM_DLLEXPORT QDir programFiles();
    #endif
    #ifdef Q_OS_MAC
        LASTFM_DLLEXPORT QDir bundle();
    #endif
        LASTFM_DLLEXPORT QDir runtimeData();
        LASTFM_DLLEXPORT QDir cache();
        LASTFM_DLLEXPORT QDir logs();
    }
   
#ifdef Q_OS_MAC 
    LASTFM_DLLEXPORT QByteArray CFStringToUtf8( CFStringRef );
    LASTFM_DLLEXPORT CFStringRef QStringToCFString( const QString& );
    LASTFM_DLLEXPORT QString CFStringToQString( CFStringRef s );
#endif
    LASTFM_DLLEXPORT const char* platform();
    LASTFM_DLLEXPORT QString md5( const QByteArray& src );
}
#endif //LASTFM_MISC_H
