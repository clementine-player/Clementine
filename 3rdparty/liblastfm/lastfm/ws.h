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
#ifndef LASTFM_WS_H
#define LASTFM_WS_H

#include "global.h"
#include <QDateTime>
#include <QMap>
#include <QNetworkReply>
#include <stdexcept>

#ifdef Q_CC_MSVC
// ms admits its lousy compiler doesn't care about throw declarations
#pragma warning( disable : 4290 )
#endif


namespace lastfm
{
    /** if you don't set one, we create our own, our own is pretty good
      * for instance, it auto detects proxy settings on windows and mac
      * We take ownership of the NAM, do not delete it out from underneath us!
      * So don't keep any other pointers to this around in case you accidently
      * call delete on them :P */
    LASTFM_DLLEXPORT void setNetworkAccessManager( QNetworkAccessManager* nam );
    LASTFM_DLLEXPORT QNetworkAccessManager* nam();

    namespace ws
    {
        LASTFM_DLLEXPORT extern QString Server;

        /** both of these are provided when you register at http://last.fm/api */
        LASTFM_DLLEXPORT extern const char* SharedSecret;
        LASTFM_DLLEXPORT extern const char* ApiKey;
    
        /** you need to set this for scrobbling to work (for now)
          * Also the User class uses it */
        LASTFM_DLLEXPORT extern QString Username;

        /** Some webservices require authentication. See the following
          * documentation:
          * http://www.last.fm/api/authentication
          * http://www.last.fm/api/desktopauth
          * You have to authenticate and then assign to SessionKey, liblastfm does
          * not do that for you. Also we do not store this. You should store this!
          * You only need to authenticate once, and that key lasts forever!
          */
        LASTFM_DLLEXPORT extern QString SessionKey;      

        enum Error
        {
            NoError = 1, // because last.fm error numbers start at 2

            /** numbers follow those at http://last.fm/api/ */
            InvalidService = 2,
            InvalidMethod,
            AuthenticationFailed,
            InvalidFormat,
            InvalidParameters,
            InvalidResourceSpecified,
            OperationFailed,
            InvalidSessionKey,
            InvalidApiKey,
            ServiceOffline,
            SubscribersOnly,

            Reserved13,
            TokenNotAuthorised,
            Reserved15,

            /** Last.fm sucks. 
              * There may be an error in networkError(), or this may just be some
              * internal error completing your request.
              * Advise the user to try again in a _few_minutes_.
              * For some cases, you may want to try again yourself, at this point
              * in the API you will have to. Eventually we will discourage this and
              * do it for you, as we don't want to strain Last.fm's servers
              */
            TryAgainLater = 16,

            Reserved17,
            Reserved18,
            Reserved19,
       
            NotEnoughContent = 20,
            NotEnoughMembers,
            NotEnoughFans,
            NotEnoughNeighbours,

            /** Last.fm fucked up, or something mangled the response on its way */
            MalformedResponse = 100,

            /** call QNetworkReply::error() as it's nothing to do with us */
            UnknownError
        };

        enum Scheme
        {
            Http,
            Https
        };

        /** Set the scheme for all web service calls. Note that it will only use
          * Https if SSL is supported on the user's machine otherwise it will
          * default to Http.
          */
        LASTFM_DLLEXPORT void setScheme( Scheme scheme );
        LASTFM_DLLEXPORT Scheme scheme();
        
        LASTFM_DLLEXPORT QString host();

        /** the map needs a method entry, as per http://last.fm/api */
        LASTFM_DLLEXPORT QUrl url( QMap<QString, QString>, bool sessionKey = true);
        LASTFM_DLLEXPORT QNetworkReply* get( QMap<QString, QString> );
        /** generates api sig, includes api key, and posts, don't add the api
          * key yourself as well--it'll break */
        LASTFM_DLLEXPORT QNetworkReply* post( QMap<QString, QString>, bool sessionKey = true );

        LASTFM_DLLEXPORT void sign( QMap<QString, QString>&, bool sessionKey = true );


        class LASTFM_DLLEXPORT ParseError
        {
        public:
            explicit ParseError( Error e, QString message );
            ParseError( const ParseError& that );
            ~ParseError() throw();
            Error enumValue() const;
            QString message() const;
            ParseError& operator=( const ParseError& that );

        private:
            class ParseErrorPrivate * const d;
        };
        
        /** returns the expiry date of this HTTP response */
        LASTFM_DLLEXPORT QDateTime expires( QNetworkReply* );
    }
}


LASTFM_DLLEXPORT QDebug operator<<( QDebug d, QNetworkReply::NetworkError e );

#define LASTFM_WS_HOSTNAME "ws.audioscrobbler.com"

#endif
