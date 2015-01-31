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
#include "Playlist.h"
#include "Track.h"
#include "ws.h"


class lastfm::PlaylistPrivate
{
    public:
        int id;
};
    

lastfm::Playlist::Playlist()
    : d( new PlaylistPrivate )
{
    d->id = -1;
}


lastfm::Playlist::Playlist( const Playlist& that )
    : d( new PlaylistPrivate( *that.d ) )
{
}


lastfm::Playlist::~Playlist()
{
    delete d;
}


lastfm::Playlist::Playlist( int id )
    : d( new PlaylistPrivate )
{
    d->id = id;
}


int
lastfm::Playlist::id() const
{
    return d->id;
}


QNetworkReply*
lastfm::Playlist::addTrack( const Track& t ) const
{
    QMap<QString, QString> map;
    map["method"] = "playlist.addTrack";
    map["playlistID"] = d->id;
    map["artist"] = t.artist();
    map["track"] = t.title();
    return lastfm::ws::post(map);
}


QNetworkReply*
lastfm::Playlist::fetch() const
{
    return fetch( QUrl("lastfm://playlist/" + QString::number( d->id )) );
}


QNetworkReply* //static
lastfm::Playlist::fetch( const QUrl& url )
{
    QMap<QString, QString> map;
    map["method"] = "playlist.fetch";
    map["playlistURL"] = url.toString();
    return lastfm::ws::get(map);
}


QNetworkReply* //static
lastfm::Playlist::create( const QString& title, const QString& description /*=""*/ )
{
    QMap<QString, QString> map;
    map["method"] = "playlist.create";
    map["title"] = title;
    if (description.size()) 
        map["description"] = description;
    return lastfm::ws::post(map);
}


lastfm::Playlist&
lastfm::Playlist::operator=( const Playlist& that )
{
    d->id = that.d->id;
    return *this;
}
