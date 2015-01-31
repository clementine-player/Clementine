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

#include <QtNetwork>
#include <QtXml>

#include "FingerprintId.h"
#include "ws.h"
#include "XmlQuery.h"
 

class lastfm::FingerprintIdPrivate
{
public:
    int id;
};


lastfm::FingerprintId::FingerprintId()
    : d( new FingerprintIdPrivate )
{
    d->id = -1;
}


lastfm::FingerprintId::FingerprintId( uint i )
    : d( new FingerprintIdPrivate )
{
    d->id = i;
}


lastfm::FingerprintId::FingerprintId( const FingerprintId& that )
    : d( new FingerprintIdPrivate( *that.d ) )
{
}


lastfm::FingerprintId::~FingerprintId()
{
    delete d;
}


bool
lastfm::FingerprintId::isNull() const
{
    return d->id == -1;
}


QNetworkReply*
lastfm::FingerprintId::getSuggestions() const
{
    QMap<QString, QString> map;
    map["method"] = "track.getFingerprintMetadata";
    map["fingerprintid"] = QString::number( d->id );
    return ws::get( map );
}


QMap<float, lastfm::Track> //static
lastfm::FingerprintId::getSuggestions( QNetworkReply* reply )
{
    QMap<float, Track> tracks;
    lastfm::XmlQuery lfm;

    if ( lfm.parse( reply ) )
    {
        foreach ( const lastfm::XmlQuery& track, lfm["tracks"].children("track") )
        {
            MutableTrack t;
            t.setTitle( track["name"].text() );
            t.setArtist( track["artist"]["name"].text() );
            t.setDuration( track["duration"].text().toInt() );
            t.setUrl( track["url"].text() );
            t.setMbid( lastfm::Mbid( track["mbid"].text() ) );
            tracks.insert( track.attribute("rank").toFloat(), t );
        }
    }

    return tracks;
}


lastfm::FingerprintId::operator int() const
{
    return d->id;
}


lastfm::FingerprintId::operator QString() const
{
    return d->id == -1 ? "" : QString::number( d->id );
}

lastfm::FingerprintId&
lastfm::FingerprintId::operator=( const FingerprintId& that )
{
    d->id = that.d->id;
    return *this;
}


QDebug
operator<<( QDebug d, lastfm::FingerprintId id)
{
    if (id.isNull())
        return d << "(null)";
    else
        return d << int(id);
}
