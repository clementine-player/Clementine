/*
   Copyright 2009-2010 Last.fm Ltd.
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
#include "Album.h"
#include "Artist.h"
#include "User.h"
#include "UrlBuilder.h"
#include "XmlQuery.h"
#include "ws.h"
#include <QFile>
#include <QStringList>
#include <QTimer>

using lastfm::Album;
using lastfm::Artist;
using lastfm::Mbid;

namespace lastfm
{
    class AlbumPrivate : public QSharedData
    {
        public:
            AlbumPrivate() {}

            Mbid mbid;
            Artist artist;
            QString title;
            QMap<AbstractType::ImageSize, QUrl> images;
    };
}

Album::Album()
    :AbstractType(), d( new lastfm::AlbumPrivate )
{
}

Album::Album( Mbid mbid )
    :AbstractType(), d( new lastfm::AlbumPrivate )
{
    d->mbid = mbid;
}

Album::Album( Artist artist, QString title )
    :AbstractType(), d( new lastfm::AlbumPrivate )
{
    d->artist = artist;
    d->title = title;
}

Album::Album( const Album& other )
    : d( other.d )
{
}

Album::~Album()
{
}

QDomElement
Album::toDomElement( QDomDocument& ) const
{
    return QDomElement();
}

QUrl
Album::imageUrl( ImageSize size, bool square ) const
{
    if( !square ) return d->images.value( size );

    QUrl url = d->images.value( size );
    QRegExp re( "/serve/(\\d*)s?/" );
    return QUrl( url.toString().replace( re, "/serve/\\1s/" ));
}

void
Album::setImageUrl( ImageSize size, const QString& url )
{
    if ( !url.isEmpty() )
        d->images[size] = url;
}

bool
Album::operator==( const Album& that ) const
{
    return d->title == that.d->title && d->artist == that.d->artist;
}

bool
Album::operator!=( const Album& that ) const
{
    return d->title != that.d->title || d->artist != that.d->artist;
}

Album&
Album::operator=( const Album& that )
{
    d = that.d;
    return *this;
}

QString
Album::toString() const
{
    return title();
}

Album::operator QString() const
{
    return toString();
}

QString
Album::title() const
{
    return d->title;
}

void
Album::setArtist( const QString& artist )
{
    return d->artist.setName( artist );
}

Artist
Album::artist() const
{
    return d->artist;
}

Mbid
Album::mbid() const
{
    return d->mbid;
}

bool
Album::isNull() const
{
    return d->title.isEmpty() && d->mbid.isNull();
}


QNetworkReply*
lastfm::Album::getInfo( const QString& username) const
{
    QMap<QString, QString> map;
    map["method"] = "album.getInfo";
    map["artist"] = d->artist;
    map["album"] = d->title;
    if (!username.isEmpty()) map["username"] = username;
    if (!lastfm::ws::SessionKey.isEmpty()) map["sk"] = lastfm::ws::SessionKey;
    return lastfm::ws::get(map);
}


QNetworkReply*
lastfm::Album::getTags() const
{
    QMap<QString, QString> map;
    map["method"] = "album.getTags";
    map["artist"] = d->artist;
    map["album"] = d->title;
    return lastfm::ws::get(map);
}


QNetworkReply*
lastfm::Album::share( const QStringList& recipients, const QString& message, bool isPublic ) const
{
    QMap<QString, QString> map;
    map["method"] = "album.share";
    map["artist"] = d->artist;
    map["album"] = d->title;
    map["recipient"] = recipients.join(",");
    map["public"] = isPublic ? "1" : "0";
    if (message.size()) map["message"] = message;
    return lastfm::ws::post(map);
}


QUrl
lastfm::Album::www() const
{
    return lastfm::UrlBuilder( "music" ).slash( d->artist ).slash( d->title ).url();
}


QNetworkReply*
lastfm::Album::addTags( const QStringList& tags ) const
{
    if (tags.isEmpty())
        return 0;

    QMap<QString, QString> map;
    map["method"] = "album.addTags";
    map["artist"] = d->artist;
    map["album"] = d->title;
    map["tags"] = tags.join( QChar(',') );
    return lastfm::ws::post(map);
}
