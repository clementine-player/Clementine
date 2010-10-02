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

#include "Song.h"

#include "Config.h"
#include "Song_p.h"
#include "AudioSummary.h"

#include <QNetworkReply>
#include <QDebug>

#include <QtNetwork/QNetworkReply>
#include "Parsing_p.h"
#include <qxmlstream.h>

Echonest::Song::Song()
    : d( new SongData )
{}

Echonest::Song::Song(const QByteArray& id, const QString& title, const QByteArray& artistId, const QString& artistName)
    :d( new SongData )
{
    d->id = id;
    d->title = title;
    d->artistId = artistId;
    d->artistName = artistName;
}

Echonest::Song::Song(const Echonest::Song& other)
    : d( other.d )
{

}


Echonest::Song::~Song()
{
    
}

Echonest::Song& Echonest::Song::operator=(const Echonest::Song& song)
{
    d = song.d;
    return *this;
}



QByteArray Echonest::Song::id() const
{
    return d->id;
}

void Echonest::Song::setId(const QByteArray& id)
{
    d->id = id;
}


QString Echonest::Song::title() const
{
    return d->title;
}

void Echonest::Song::setTitle(const QString& title)
{
    d->title = title;
}

QByteArray Echonest::Song::artistId() const
{
    return d->artistId;
}

void Echonest::Song::setArtistId(const QByteArray& artistId)
{
    d->artistId = artistId;
}

QString Echonest::Song::artistName() const
{
    return d->artistName;
}

void Echonest::Song::setArtistName(const QString& artistName)
{
    d->artistName = artistName;
}

QVector< Echonest::Track > Echonest::Song::tracks() const
{
    return d->tracks;
}

void Echonest::Song::setTracks(const QVector< Echonest::Track >& tracks)
{
    d->tracks = tracks;
}

qreal Echonest::Song::hotttnesss() const
{
    return d->hotttnesss;
}

void Echonest::Song::setHotttnesss(qreal hotttnesss)
{
    d->hotttnesss = hotttnesss;
}

qreal Echonest::Song::artistHotttnesss() const
{
    return d->artistHotttnesss;
}

void Echonest::Song::setArtistHotttnesss(qreal artistHotttnesss)
{
    d->artistHotttnesss = artistHotttnesss;
}

Echonest::AudioSummary Echonest::Song::audioSummary() const
{
    return d->audioSummary;
}

void Echonest::Song::setAudioSummary(const Echonest::AudioSummary& summary)
{
    d->audioSummary = summary;
}

qreal Echonest::Song::artistFamiliarity() const
{
    return d->artistFamiliarity;
}

void Echonest::Song::setArtistFamiliarity(qreal artistFamiliarity)
{
    d->artistFamiliarity = artistFamiliarity;
}

QString Echonest::Song::artistLocation() const
{
    return d->artistLocation;
}

void Echonest::Song::setArtistLocation(const QString& artistLocation)
{
    d->artistLocation = artistLocation;
}

QNetworkReply* Echonest::Song::fetchInformation( Echonest::Song::SongInformation parts ) const
{
    QUrl url = Echonest::baseGetQuery( "song", "profile" );
    url.addEncodedQueryItem( "id", d->id );
    addQueryInformation( url, parts );
    
    qDebug() << "Creating fetchInformation URL" << url;
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Song::search( const Echonest::Song::SearchParams& params, Echonest::Song::SongInformation parts )
{
    QUrl url = Echonest::baseGetQuery( "song", "search" );
    addQueryInformation( url, parts );
    
    SearchParams::const_iterator iter = params.constBegin();
    for( ; iter < params.constEnd(); ++iter )
        url.addQueryItem( QLatin1String( searchParamToString( iter->first ) ), iter->second.toString() );
    
    qDebug() << "Creating search URL" << url;
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

void Echonest::Song::parseInformation( QNetworkReply* reply ) throw( ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    
    QXmlStreamReader xml( reply->readAll() );
    
    Echonest::Parser::readStatus( xml );
    // we'll just take the new data. it is given as a list even though it can only have 1 song as we specify the song id
    QVector< Echonest::Song > songs = Echonest::Parser::parseSongList( xml );
    if( !songs.size() == 1 ) { // no data for this song. returned empty.
        return;
    }
    // copy any non-default values
    Echonest::Song newSong = songs.at( 0 );
    if( newSong.hotttnesss() >= 0 )
        setHotttnesss( newSong.hotttnesss() );
    if( newSong.artistHotttnesss() >= 0 )
        setArtistHotttnesss( newSong.artistHotttnesss() );
    if( newSong.artistFamiliarity() >= 0 )
        setArtistFamiliarity( newSong.artistFamiliarity() );
    if( !newSong.artistLocation().isEmpty() )
        setArtistLocation( newSong.artistLocation() );
    
}

QVector< Echonest::Song > Echonest::Song::parseSearch( QNetworkReply* reply ) throw( ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    
    QXmlStreamReader xml( reply->readAll() );
    
    Echonest::Parser::readStatus( xml );
    QVector<Echonest::Song> songs = Echonest::Parser::parseSongList( xml );

    return songs;
    
}

QByteArray Echonest::Song::searchParamToString( Echonest::Song::SearchParam param )
{
    switch( param )
    {
        case Echonest::Song::Title:
            return "title";
        case Echonest::Song::Artist:
            return "artist";
        case Echonest::Song::Combined:
            return "combined";
        case Echonest::Song::Description:
            return "description";
        case Echonest::Song::ArtistId:
            return "artist_id";
        case Echonest::Song::Results:
            return "results";
        case Echonest::Song::MaxTempo:
            return "max_tempo";
        case Echonest::Song::MinTempo:
            return "min_tempo";
        case Echonest::Song::MaxDanceability:
            return "max_danceability";
        case Echonest::Song::MinDanceability:
            return "min_danceability";
        case Echonest::Song::MaxComplexity:
            return "max_complexity";
        case Echonest::Song::MinComplexity:
            return "min_complexity";
        case Echonest::Song::MaxDuration:
            return "max_duration";
        case Echonest::Song::MinDuration:
            return "min_duration";
        case Echonest::Song::MaxLoudness:
            return "max_loudness";
        case Echonest::Song::MinLoudness:
            return "min_loudness";
        case Echonest::Song::MaxFamiliarity:
            return "max_familiarity";
        case Echonest::Song::MinFamiliarity:
            return "min_familiarity";
        case Echonest::Song::MaxHotttnesss:
            return "max_hotttnesss";
        case Echonest::Song::MinHotttnesss:
            return "min_hotttnesss";
        case Echonest::Song::MaxLongitude:
            return "max_longitude";
        case Echonest::Song::MinLongitude:
            return "min_longitude";
        case Echonest::Song::Mode:
            return "mode";
        case Echonest::Song::Key:
            return "key";
        case Echonest::Song::Sort:
            return "sort";
    }
    return QByteArray();
}

void Echonest::Song::addQueryInformation(QUrl& url, Echonest::Song::SongInformation parts)
{
    if( parts.testFlag( Echonest::Song::AudioSummaryInformation ) )
        url.addEncodedQueryItem( "bucket", "audio_summary" );
    if( parts.testFlag( Echonest::Song::Tracks ) )
        url.addEncodedQueryItem( "bucket", "tracks" );
    if( parts.testFlag( Echonest::Song::Hotttnesss ) )
        url.addEncodedQueryItem( "bucket", "song_hotttnesss" );
    if( parts.testFlag( Echonest::Song::ArtistHotttnesss ) )
        url.addEncodedQueryItem( "bucket", "artist_hotttnesss" );
    if( parts.testFlag( Echonest::Song::ArtistFamiliarity ) )
        url.addEncodedQueryItem( "bucket", "artist_familiarity" );
    if( parts.testFlag( Echonest::Song::ArtistLocation ) )
        url.addEncodedQueryItem( "bucket", "artist_location" );
}


QString Echonest::Song::toString() const
{
    return QString::fromLatin1( "Song(%1, %2, %3, %4)" ).arg( title() ).arg( artistName() ).arg( QString::fromLatin1( id() ) ).arg( QString::fromLatin1( artistId() ) );
}


QDebug Echonest::operator<<(QDebug d, const Echonest::Song& song)
{
    d << song.toString();
    return d.maybeSpace();
}

