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

#include "Library.h"
#include "ws.h"

#include <QNetworkReply>


lastfm::Library::Library()
{
}

QNetworkReply*
lastfm::Library::addAlbum( const QList<lastfm::Album>& albums )
{
    QMap<QString, QString> map;
    map["method"] = "library.addAlbum";

    for ( int i = 0 ; i < albums.count() ; ++i )
    {
        map["artist[" + QString::number(i) + "]"] = albums[i].artist();
        map["album[" + QString::number(i) + "]"] = albums[i].title();
    }

    return ws::post( map );
}

QNetworkReply*
lastfm::Library::addArtist( const QList<lastfm::Artist>& artists )
{
    QMap<QString, QString> map;
    map["method"] = "library.addArtist";

    for ( int i = 0 ; i < artists.count() ; ++i )
        map["artist[" + QString::number(i) + "]"] = artists[i].name();

    return ws::post( map );
}

QNetworkReply*
lastfm::Library::addTrack( const Track &track )
{
    QMap<QString, QString> map;
    map["method"] = "library.addTrack";
    map["track"] = track.title();
    map["artist"] = track.artist().name();
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::getAlbums( const QString& user, const lastfm::Artist& artist, int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "library.getAlbums";
    map["user"] = user;
    map["artist"] = artist.name();
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::getArtists( const QString& user, int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "library.getArtists";
    map["user"] = user;
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::getTracks( const QString& user, const lastfm::Artist& artist, int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "library.getTracks";
    map["user"] = user;
    map["artist"] = artist.name();
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::getTracks( const QString& user, const lastfm::Album& album, int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "library.getTracks";
    map["user"] = user;
    map["album"] = album.title();
    map["artist"] = album.artist().name();
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::removeAlbum( const lastfm::Album& album )
{
    QMap<QString, QString> map;
    map["method"] = "library.removeAlbum";
    map["album"] = album.title();
    map["artist"] = album.artist().name();
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::removeArtist( const lastfm::Artist& artist )
{
    QMap<QString, QString> map;
    map["method"] = "library.removeArtist";
    map["artist"] = artist.name();
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::removeTrack( const lastfm::Track& track )
{
    QMap<QString, QString> map;
    map["method"] = "library.removeTrack";
    map["artist"] = track.artist().name();
    map["track"] = track.title();
    return ws::post( map );
}

QNetworkReply*
lastfm::Library::removeScrobble( const lastfm::Track& track )
{
    QMap<QString, QString> map;
    map["method"] = "library.removeScrobble";
    map["artist"] = track.artist().name();
    map["track"] = track.title();
    map["timestamp"] = QString::number( track.timestamp().toTime_t() );
    return ws::post( map );
}

