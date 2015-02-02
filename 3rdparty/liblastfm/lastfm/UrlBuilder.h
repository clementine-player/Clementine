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
#ifndef LASTFM_URL_BUILDER_H
#define LASTFM_URL_BUILDER_H

#include "global.h"
#include <QLocale>
#include <QUrl>
  

namespace lastfm
{
    /** For building www.last.fm urls. We have special rules for encoding and that */
    class LASTFM_DLLEXPORT UrlBuilder
    {
    public:
        /** Careful, the base is not encoded at all, we assume it is ASCII!
          * If you need it encoded at all you must use the slash function.
          * eg. UrlBuilder( "user" ).slash( "mxcl" ) ==> http://last.fm/user/mxcl
          */
        UrlBuilder( const QString& base );

        UrlBuilder& slash( const QString& path );

        UrlBuilder( const UrlBuilder& that );
        ~UrlBuilder();

        QUrl url() const;

        /** www.last.fm becomes the local version, eg www.lastfm.de */
        static QUrl localize( QUrl );
        /** www.last.fm becomes m.last.fm, localisation is preserved */
        static QUrl mobilize( QUrl );

        /** Use this to URL encode any database item (artist, track, album). It
          * internally calls UrlEncodeSpecialChars to double encode some special
          * symbols according to the same pattern as that used on the website.
          *
          * &, /, ;, +, #
          *
          * Use for any urls that go to www.last.fm
          * Do not use for ws.audioscrobbler.com
          */
        static QByteArray encode( QString );

        /** returns eg. www.lastfm.de */
        static QString host( const QLocale& = QLocale() );

        /** return true if url is a last.fm url */
        static bool isHost( const QUrl& url );

        UrlBuilder& operator=( const UrlBuilder& that );

    private:
        class UrlBuilderPrivate * const d;
    };
}

#endif
