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
#include "ws.h"
#include "misc.h"
#include "NetworkAccessManager.h"
#include "Url.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QLocale>
#include <QStringList>
#include <QUrl>
#include <QThread>
#include <QMutex>
#include <QSslSocket>

static lastfm::ws::Scheme theScheme = lastfm::ws::Http;
static QMap< QThread*, QNetworkAccessManager* > threadNamHash;
static QSet< QThread* > ourNamSet;
static QMutex namAccessMutex;

class lastfm::ws::ParseErrorPrivate
{
public:
    lastfm::ws::Error e;
    QString m_message;
};

lastfm::ws::ParseError::ParseError( lastfm::ws::Error e, QString message )
    :d( new ParseErrorPrivate )
{
     d->e = e;
     d->m_message = message;
}

lastfm::ws::ParseError::ParseError( const ParseError& that )
    : d( new ParseErrorPrivate( *that.d ) )
{
}

lastfm::ws::ParseError::~ParseError() throw()
{
    delete d;
}

lastfm::ws::Error
lastfm::ws::ParseError::enumValue() const
{
    return d->e;
}

QString
lastfm::ws::ParseError::message() const
{
    return d->m_message;
}

lastfm::ws::ParseError&
lastfm::ws::ParseError::operator=( const ParseError& that )
{
    d->e = that.d->e;
    d->m_message = that.d->m_message;
    return *this;
}

lastfm::ws::Scheme
lastfm::ws::scheme()
{
    return QSslSocket::supportsSsl() ? theScheme : Http;
}

void
lastfm::ws::setScheme( lastfm::ws::Scheme scheme )
{
    theScheme = scheme;
}

QString
lastfm::ws::host()
{
    QStringList const args = QCoreApplication::arguments();
    if (args.contains( "--debug"))
        return "ws.staging.audioscrobbler.com";

    int const n = args.indexOf( "--host" );
    if (n != -1 && args.count() > n+1)
        return args[n+1];

    return LASTFM_WS_HOSTNAME;
}

static QUrl baseUrl()
{
    QUrl url;
    url.setScheme( lastfm::ws::scheme() == lastfm::ws::Https ? "https" : "http" );
    url.setHost( lastfm::ws::host() );
    url.setPath( "/2.0/" );

    return url;
}


static QUrl baseUrl(const QString& host)
{
    if(host.isEmpty())
      return baseUrl();

    QUrl url( host + "/2.0/" );

    return url;
}

static QString iso639()
{
    return QLocale().name().left( 2 ).toLower();
}

void autograph( QMap<QString, QString>& params )
{
    params["api_key"] = lastfm::ws::ApiKey;
    params["lang"] = iso639();
}

void
lastfm::ws::sign( QMap<QString, QString>& params, bool sk )
{
    autograph( params );
    // it's allowed for sk to be null if we this is an auth call for instance
    if (sk && lastfm::ws::SessionKey.size())
        params["sk"] = lastfm::ws::SessionKey;

    QString s;
    QMapIterator<QString, QString> i( params );
    while (i.hasNext()) {
        i.next();
        s += i.key() + i.value();
    }
    s += lastfm::ws::SharedSecret;

    params["api_sig"] = lastfm::md5( s.toUtf8() );
}


QUrl
lastfm::ws::url( QMap<QString, QString> params, bool sk )
{
    lastfm::ws::sign( params, sk );
    lastfm::Url url = lastfm::Url( ::baseUrl(lastfm::ws::Server) );
    // Qt setQueryItems doesn't encode a bunch of stuff, so we do it manually
    QMapIterator<QString, QString> i( params );
    while (i.hasNext()) {
        i.next();
        QString const key = i.key();
        QString const value = i.value();
        url.addQueryItem( key, value );
    }

    return url.url();
}


QNetworkReply*
lastfm::ws::get( QMap<QString, QString> params )
{
    return nam()->get( QNetworkRequest( url( params ) ) );
}


QNetworkReply*
lastfm::ws::post( QMap<QString, QString> params, bool sk )
{
    sign( params, sk );
    QByteArray query;
    QMapIterator<QString, QString> i( params );
    while (i.hasNext()) {
        i.next();
        query += QUrl::toPercentEncoding( i.key() )
               + '='
               + QUrl::toPercentEncoding( i.value() )
               + '&';
    }
    QNetworkRequest req( baseUrl(lastfm::ws::Server) );

    req.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
    return nam()->post( req, query );
}


QNetworkAccessManager*
lastfm::nam()
{
    QMutexLocker l( &namAccessMutex );
    QThread* thread = QThread::currentThread();
    if ( !threadNamHash.contains( thread ) )
    {
        NetworkAccessManager* newNam = new NetworkAccessManager();
        threadNamHash[thread] = newNam;
        ourNamSet.insert( thread );
        return newNam;
    }
    return threadNamHash[thread];
}


void
lastfm::setNetworkAccessManager( QNetworkAccessManager* nam )
{
    if ( !nam )
        return;

    QMutexLocker l( &namAccessMutex );
    QThread* thread = QThread::currentThread();
    QNetworkAccessManager* oldNam = 0;
    if ( threadNamHash.contains( thread ) && ourNamSet.contains( thread ) )
        oldNam = threadNamHash[thread];

    if ( oldNam == nam )
    {
        // If we're being passed back our own NAM, assume they want to
        // ensure that we don't delete it out from under them
        ourNamSet.remove( thread );
        return;
    }

    threadNamHash[thread] = nam;
    ourNamSet.remove( thread );

    if ( oldNam )
        delete oldNam;
}


/** This useful function, fromHttpDate, comes from QNetworkHeadersPrivate
  * in qnetworkrequest.cpp.  Qt copyright and license apply. */
static QDateTime QByteArrayToHttpDate(const QByteArray &value)
{
    // HTTP dates have three possible formats:
    //  RFC 1123/822      -   ddd, dd MMM yyyy hh:mm:ss "GMT"
    //  RFC 850           -   dddd, dd-MMM-yy hh:mm:ss "GMT"
    //  ANSI C's asctime  -   ddd MMM d hh:mm:ss yyyy
    // We only handle them exactly. If they deviate, we bail out.

    int pos = value.indexOf(',');
    QDateTime dt;
    if (pos == -1) {
        // no comma -> asctime(3) format
        dt = QDateTime::fromString(QString::fromLatin1(value), Qt::TextDate);
    } else {
        // eat the weekday, the comma and the space following it
        QString sansWeekday = QString::fromLatin1(value.constData() + pos + 2);

        QLocale c = QLocale::c();
        if (pos == 3)
            // must be RFC 1123 date
            dt = c.toDateTime(sansWeekday, QLatin1String("dd MMM yyyy hh:mm:ss 'GMT"));
        else
            // must be RFC 850 date
            dt = c.toDateTime(sansWeekday, QLatin1String("dd-MMM-yy hh:mm:ss 'GMT'"));
    }

    if (dt.isValid())
        dt.setTimeSpec(Qt::UTC);
    return dt;
}


QDateTime
lastfm::ws::expires( QNetworkReply* reply )
{
    return QByteArrayToHttpDate( reply->rawHeader( "Expires" ) );
}


namespace lastfm
{
    namespace ws
    {
        QString Server;
        QString SessionKey;
        QString Username;

        /** we leave these unset as you can't use the webservices without them
          * so lets make the programmer aware of it during testing by crashing */
        const char* SharedSecret;
        const char* ApiKey;

        /** if this is found set to "" we conjure ourselves a suitable one */
        const char* UserAgent = 0;
    }
}


QDebug operator<<( QDebug, lastfm::ws::Error );

QDebug operator<<( QDebug d, QNetworkReply::NetworkError e )
{
    return d << lastfm::qMetaEnumString<QNetworkReply>( e, "NetworkError" );
}
