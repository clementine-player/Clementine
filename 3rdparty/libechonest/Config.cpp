/****************************************************************************************
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>                                    *
 * Copyright (c) 2011 Jeff Mitchell <mitchell@kde.org>                                  *
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
#include <QThread>
#include <QDebug>
#include <QMutex>

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
    whatData = createWhatData();
}

Echonest::ParseError::ParseError(Echonest::ErrorType error, const QString& text): exception()
{
    type =  error;
    extraText = text;
    whatData = createWhatData();
}

QByteArray Echonest::ParseError::createWhatData() const throw()
{
    // If we have specific error text, use that first
    if( !extraText.isEmpty() )
        return extraText.toLatin1();

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
    return QByteArray();
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
    return whatData.constData();
}


QNetworkReply::NetworkError Echonest::ParseError::networkError() const throw()
{
    return nError;
}



class Echonest::ConfigPrivate {
public:
    ConfigPrivate()
    {
        threadNamHash[ QThread::currentThread() ] = new QNetworkAccessManager();
    }

    ~ConfigPrivate()
    {
        QThread *currThread = QThread::currentThread();
        if( threadNamHash.contains( currThread ) )
        {
            if ( ourNamSet.contains( currThread ) )
                delete threadNamHash[ currThread ];
            threadNamHash.remove( currThread );
            ourNamSet.remove( currThread );
        }
    }

    QMutex accessMutex;
    QHash< QThread*, QNetworkAccessManager* > threadNamHash;
    QSet< QThread* > ourNamSet;
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

    QMutexLocker l( &d->accessMutex );
    QThread* currThread = QThread::currentThread();
    QNetworkAccessManager* oldNam = 0;
    if( d->threadNamHash.contains( currThread ) && d->ourNamSet.contains( currThread ) )
        oldNam = d->threadNamHash[ currThread ];

    if( oldNam == nam )
    {
        // If we're being passed back our own NAM, assume they want to
        // ensure that we don't delete it out from under them 
        d->ourNamSet.remove( currThread );
        return;
    }

    d->threadNamHash[ currThread ] = nam;
    d->ourNamSet.remove( currThread );

    if( oldNam )
        delete oldNam;
}

QNetworkAccessManager* Echonest::Config::nam() const
{
    QMutexLocker l( &d->accessMutex );
    QThread* currThread = QThread::currentThread();
    if( !d->threadNamHash.contains( currThread ) )
    {
        QNetworkAccessManager *newNam = new QNetworkAccessManager();
        d->threadNamHash[ currThread ] = newNam;
        d->ourNamSet.insert( currThread );
        return newNam;
    }

    return d->threadNamHash[ currThread ];
}

Echonest::Config* Echonest::Config::instance() {
    if ( !s_instance ) {
        s_instance = new Config;
    }

    return s_instance;
}
