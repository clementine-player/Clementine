/****************************************************************************************
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>                                    *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "Config.h"

#include <QNetworkAccessManager>
#include <QDebug>

Echonest::Config* Echonest::Config::s_instance = 0;

QUrl Echonest::baseUrl()
{
    QUrl url;
    url.setScheme( QLatin1String( "http" ) );
    url.setHost( QLatin1String( "developer.echonest.com" ) );
    return url;
}


QUrl Echonest::baseGetQuery(const QByteArray& type, const QByteArray& method)
{
    QUrl url = baseUrl();
    url.setPath( QString::fromLatin1( "/api/v4/%1/%2" ).arg( QLatin1String( type ) ).arg( QLatin1String( method ) ) );
    url.addEncodedQueryItem( "api_key", Echonest::Config::instance()->apiKey() );
    url.addEncodedQueryItem( "format", "xml" );
    return url;
}

Echonest::ParseError::ParseError(Echonest::ErrorType error): exception()
{
    type = error;
}

Echonest::ParseError::~ParseError() throw()
{}

Echonest::ErrorType Echonest::ParseError::errorType() const throw()
{
    return type;
}

void Echonest::ParseError::setNetworkError( QNetworkReply::NetworkError error ) throw()
{
    nError = error;
}

const char* Echonest::ParseError::what() const throw()
{
    switch( type )
    {
        case UnknownError:
            return "Unknown Echo Nest Error";
        case NoError:
            return "No Error";
        case MissingAPIKey:
            return "Missing Echo Nest API Key";
        case NotAllowed:
            return "Method not allowed";
        case RateLimitExceeded:
            return "Rate limit exceeded";
        case MissingParameter:
            return "Missing parameter";
        case InvalidParameter:
            return "Invalid parameter";
        case UnfinishedQuery:
            return "Unfinished query object";
        case EmptyResult:
            return "No results";
        case NetworkError:
            return "Network Error";
        case UnknownParseError:
            return "Unknown Parse Error";
    }
    return "";
}


QNetworkReply::NetworkError Echonest::ParseError::networkError() const throw()
{
    return nError;
}



class Echonest::ConfigPrivate {
public:
    ConfigPrivate() : nam( new QNetworkAccessManager )
    {
    }
    
    ~ConfigPrivate()
    {
        delete nam;
        nam = 0;
    }
    
    QNetworkAccessManager* nam;
    QByteArray apikey;
};

Echonest::Config::Config()
    : d( new Echonest::ConfigPrivate )
{

}

Echonest::Config::~Config()
{
    delete d;
}

void Echonest::Config::setAPIKey(const QByteArray& apiKey)
{
    d->apikey = apiKey;
}

QByteArray Echonest::Config::apiKey() const
{
    return d->apikey;
}


void Echonest::Config::setNetworkAccessManager(QNetworkAccessManager* nam)
{
    if( !nam )
        return;
    
    if( d->nam ) {
        delete d->nam;
    }
    d->nam = nam;
}

QNetworkAccessManager* Echonest::Config::nam() const
{
    return d->nam;
}

Echonest::Config* Echonest::Config::instance() {
    if ( !s_instance ) {
        s_instance = new Config;
    }

    return s_instance;
}
