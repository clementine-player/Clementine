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
#include "UrlBuilder.h"
#include <QRegExp>
#include <QStringList>


class lastfm::UrlBuilderPrivate
{
public:
    QByteArray path;
};


lastfm::UrlBuilder::UrlBuilder( const QString& base )
    : d( new UrlBuilderPrivate )
{
    d->path = '/' + base.toLatin1();
}


lastfm::UrlBuilder::UrlBuilder( const UrlBuilder& that )
    : d( new UrlBuilderPrivate( *that.d ) )
{
}


lastfm::UrlBuilder::~UrlBuilder()
{
    delete d;
}


lastfm::UrlBuilder&
lastfm::UrlBuilder::slash( const QString& path )
{
    this->d->path += '/' + encode( path );
    return *this;
}


QUrl
lastfm::UrlBuilder::url() const
{
    QUrl url;
    url.setScheme( "http" );
    url.setHost( host() );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    url.setPath( d->path );
#else
    url.setEncodedPath( d->path );
#endif
    return url;
}


QByteArray //static
lastfm::UrlBuilder::encode( QString s )
{
    foreach (QChar c, QList<QChar>() << '%' << '&' << '/' << ';' << '+' << '#' << '"')
        if (s.contains( c ))
            // the middle step may seem odd but this is what the site does
            // eg. search for the exact string "Radiohead 2 + 2 = 5"
            return QUrl::toPercentEncoding( s ).replace( "%20", "+" ).toPercentEncoding( "", "+" );;

    return QUrl::toPercentEncoding( s.replace( ' ', '+' ), "+" );
}


QString //static
lastfm::UrlBuilder::host( const QLocale& locale )
{
    switch (locale.language())
    {
        case QLocale::Portuguese: return "www.lastfm.com.br";
        case QLocale::Turkish:    return "www.lastfm.com.tr";
        case QLocale::French:     return "www.lastfm.fr";
        case QLocale::Italian:    return "www.lastfm.it";
        case QLocale::German:     return "www.lastfm.de";
        case QLocale::Spanish:    return "www.lastfm.es";
        case QLocale::Polish:     return "www.lastfm.pl";
        case QLocale::Russian:    return "www.lastfm.ru";
        case QLocale::Japanese:   return "www.lastfm.jp";
        case QLocale::Swedish:    return "www.lastfm.se";
        case QLocale::Chinese:    return "cn.last.fm";
        default:                  return "www.last.fm";
    }
}


bool // static
lastfm::UrlBuilder::isHost( const QUrl& url )
{
    QStringList hosts = QStringList() << "www.lastfm.com.br"
                    <<  "www.lastfm.com.tr"
                    << "www.lastfm.fr"
                    << "www.lastfm.it"
                    << "www.lastfm.de"
                    << "www.lastfm.es"
                    << "www.lastfm.pl"
                    << "www.lastfm.ru"
                    << "www.lastfm.jp"
                    << "www.lastfm.se"
                    << "cn.last.fm"
                    << "www.last.fm";

    return hosts.contains( url.host() );
}

QUrl //static
lastfm::UrlBuilder::localize( QUrl url)
{
    url.setHost( url.host().replace( QRegExp("^(www.)?last.fm"), host() ) );
    return url;
}


QUrl //static
lastfm::UrlBuilder::mobilize( QUrl url )
{
    url.setHost( url.host().replace( QRegExp("^(www.)?last"), "m.last" ) );
    return url;
}

lastfm::UrlBuilder&
lastfm::UrlBuilder::operator=( const UrlBuilder& that )
{
    d->path = that.d->path;
    return *this;
}
