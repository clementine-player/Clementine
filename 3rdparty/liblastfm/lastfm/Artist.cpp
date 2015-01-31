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

#include "Artist.h"
#include "User.h"
#include "UrlBuilder.h"
#include "XmlQuery.h"
#include "ws.h"

#include <QDebug>
#include <QRegExp>
#include <QStringList>

using lastfm::Artist;
using lastfm::ArtistData;
using lastfm::User;
using lastfm::XmlQuery;


class lastfm::ArtistData : public QSharedData
{
public:
    ArtistData() {}
    ~ArtistData() {}
    QString name;
    QMap<AbstractType::ImageSize, QUrl> images;
    QString biography;
    QString biographySummary;
};


Artist::Artist()
    :AbstractType()
{
    d = new ArtistData;
}

Artist::Artist( const QString& name )
    : AbstractType()
{
    d = new ArtistData;
    d->name = name;
}

Artist::Artist( const XmlQuery& xml )
    :AbstractType()
{
    d = new ArtistData;

    d->name = xml["name"].text();
    setImageUrl( SmallImage, xml["image size=small"].text() );
    setImageUrl( MediumImage, xml["image size=medium"].text() );
    setImageUrl( LargeImage, xml["image size=large"].text() );
    setImageUrl( ExtraLargeImage, xml["image size=extralarge"].text() );
    setImageUrl( MegaImage, xml["image size=mega"].text() );

    d->biography = xml["bio"]["content"].text().trimmed();
    d->biographySummary = xml["bio"]["summary"].text().trimmed();
}


Artist::Artist( const Artist& artist )
    :AbstractType(), d( artist.d )
{
}


Artist::~Artist()
{
}


QUrl
Artist::imageUrl( ImageSize size, bool square ) const
{
    if( !square ) return d->images.value( size );

    QUrl url = d->images.value( size );
    QRegExp re( "/serve/(\\d*)s?/" );
    return QUrl( url.toString().replace( re, "/serve/\\1s/" ));
}


void
Artist::setImageUrl( ImageSize size, const QString& url )
{
    if ( !url.isEmpty() )
        d->images[size] = url;
}


QString
Artist::biography() const
{
    return d->biography;
}


QString
Artist::biographySummary() const
{
    return d->biographySummary;
}


QMap<QString, QString> //private
Artist::params( const QString& method ) const
{
    QMap<QString, QString> map;
    map["method"] = "artist."+method;
    map["artist"] = d->name;
    return map;
}


QNetworkReply*
Artist::share( const QStringList& recipients, const QString& message, bool isPublic ) const
{
    QMap<QString, QString> map = params("share");
    map["recipient"] = recipients.join(",");
    map["public"] = isPublic ? "1" : "0";
    if (message.size()) map["message"] = message;
    return lastfm::ws::post(map);
}


QUrl
Artist::www() const
{
    return UrlBuilder( "music" ).slash( Artist::name() ).url();
}


QNetworkReply*
Artist::getEvents(int limit) const
{
    QMap<QString, QString> map = params("getEvents");
    if (limit) map["limit"] = QString::number(limit);
    return ws::get( map );
}


QNetworkReply*
Artist::getInfo( const QString& username ) const
{
    QMap<QString, QString> map = params("getInfo");
    if (!username.isEmpty()) map["username"] = username;
    if (!lastfm::ws::SessionKey.isEmpty()) map["sk"] = lastfm::ws::SessionKey;
    return ws::get( map );
}


QNetworkReply*
Artist::getTags() const
{
    return ws::get( params("getTags") );
}


QNetworkReply*
Artist::getTopTags() const
{
    return ws::get( params("getTopTags") );
}


QNetworkReply*
Artist::getTopTracks() const
{
    return ws::get( params("getTopTracks") );
}


QNetworkReply*
Artist::getSimilar( int limit ) const
{
    QMap<QString, QString> map = params("getSimilar");
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}


QNetworkReply*
Artist::search( int limit ) const
{
    QMap<QString, QString> map = params("search");
    if (limit > 0) map["limit"] = QString::number(limit);
    return ws::get(map);
}


QMap<int, QString> /* static */
Artist::getSimilar( QNetworkReply* r )
{
    QMap<int, QString> artists;

    XmlQuery lfm;

    if ( lfm.parse( r ) )
    {
        foreach (XmlQuery e, lfm.children( "artist" ))
        {
            // convert floating percentage to int in range 0 to 10,000
            int const match = e["match"].text().toFloat() * 100;
            artists.insertMulti( match, e["name"].text() );
        }
    }
    else
    {
        qWarning() << lfm.parseError().message();
    }
    return artists;
}


QStringList /* static */
Artist::getTopTracks( QNetworkReply* r )
{
    QStringList tracks;
    try
    {
        XmlQuery lfm;
        lfm.parse( r );
        foreach (XmlQuery e, lfm.children( "track" ))
        {
            tracks << e["name"].text();
        }
    }
    catch (ws::ParseError& e)
    {
        qWarning() << e.message();
    }
    return tracks;
}


QList<Artist> /* static */
Artist::list( QNetworkReply* r )
{
    QList<Artist> artists;
    XmlQuery lfm;

    if ( lfm.parse( r ) )
    {
        foreach (XmlQuery xq, lfm.children( "artist" ))
        {
            Artist artist( xq );
            artists += artist;
        }
    }
    else
    {
        qWarning() << lfm.parseError().message();
    }
    return artists;
}


Artist
Artist::getInfo( QNetworkReply* r )
{
    XmlQuery lfm;

    if ( lfm.parse( r ) )
    {
        Artist artist = Artist( lfm["artist"] );
        return artist;
    }
    else
    {
        qWarning() << lfm.parseError().message();
        return Artist();
    }
}


QNetworkReply*
Artist::addTags( const QStringList& tags ) const
{
    if (tags.isEmpty())
        return 0;
    QMap<QString, QString> map = params("addTags");
    map["tags"] = tags.join( QChar(',') );
    return ws::post(map);
}


bool
Artist::isNull() const
{
    return d->name.isEmpty();
}


Artist&
Artist::operator=( const Artist& that )
{
    d->name = that.name(); d->images = that.d->images; return *this;
}


bool
lastfm::Artist::operator==( const Artist& that ) const
{
    return d->name == that.d->name;
}


bool
Artist::operator!=( const Artist& that ) const
{
    return d->name != that.d->name;
}


bool
Artist::operator<( const Artist& that ) const
{
    return d->name < that.d->name;
}


Artist::operator QString() const
{
    return d->name;
}


QString Artist::toString() const
{
    return name();
}


QString Artist::name() const
{
    return QString(*this);
}


void Artist::setName( const QString& name )
{
    d->name = name;
}


QDomElement Artist::toDomElement( QDomDocument& ) const
{
    return QDomElement();
}

