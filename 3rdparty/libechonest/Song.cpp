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
#include <qjson/serializer.h>
#include <qjson/parser.h>

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

Echonest::Song::Song(const QByteArray& id)
:d( new SongData )
{
    d->id = id;
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

QString Echonest::Song::release() const
{
    return d->release;
}

void Echonest::Song::setRelease(const QString& release)
{
    d->release = release;
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

Echonest::ArtistLocation Echonest::Song::artistLocation() const
{
    return d->artistLocation;
}

void Echonest::Song::setArtistLocation(const Echonest::ArtistLocation& artistLocation)
{
    d->artistLocation = artistLocation;
}

QNetworkReply* Echonest::Song::fetchInformation( Echonest::SongInformation information ) const
{
    QUrl url = Echonest::baseGetQuery( "song", "profile" );
    url.addEncodedQueryItem( "id", d->id );
    addQueryInformation( url, information );

    qDebug() << "Creating fetchInformation URL" << url;
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Song::search( const Echonest::Song::SearchParams& params, Echonest::SongInformation information )
{
    QUrl url = Echonest::baseGetQuery( "song", "search" );
    addQueryInformation( url, information );

    SearchParams::const_iterator iter = params.constBegin();
    for( ; iter < params.constEnd(); ++iter )
        url.addEncodedQueryItem( searchParamToString( iter->first ), Echonest::escapeSpacesAndPluses( iter->second.toString() ) );

    qDebug() << "Creating search URL" << url;
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Song::identify( const Echonest::Song::IdentifyParams& params, const Echonest::SongInformation& information )
{
    QVariantMap query;
    QVariantMap metadata;
    IdentifyParams::const_iterator iter = params.constBegin();
    for( ; iter < params.constEnd(); ++iter ) {
        if( iter->first == Code )
            query[ QLatin1String( identifyParamToString( iter->first ) ) ] = iter->second;
        else
            metadata[ QLatin1String( identifyParamToString( iter->first ) ) ] =  iter->second.toString();
    }
    metadata[ QLatin1String( "version" ) ] = QLatin1String( "4.12" );
    query[ QLatin1String( "metadata" ) ] = metadata;
    QJson::Serializer s;
    QByteArray data = s.serialize( query );

    QUrl url = Echonest::baseGetQuery( "song", "identify" );
    addQueryInformation( url, information );

    qDebug() << "Creating identify URL" << url;
    QNetworkRequest request( url );

    request.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/octet-stream" ) );
    //     qDebug() << "Uploading local file to" << url;
    return Echonest::Config::instance()->nam()->post( request, data );
}

Echonest::SongList Echonest::Song::parseIdentify( QNetworkReply* reply ) throw( ParseError )
{
    Echonest::Parser::checkForErrors( reply );

    QByteArray data = reply->readAll();
    QJson::Parser p;
    QVariantMap res = p.parse( data ).toMap();
//     qDebug() << "Got data from identify call:" << data << res;


    qDebug() << res[ QLatin1String( "response" ) ].toMap()[ QLatin1String( "songs" ) ].toList();
    if( !res.contains( QLatin1String( "response" ) ) || !res[ QLatin1String( "response" ) ].toMap().contains( QLatin1String( "songs" ) ) ) {
        qDebug() << "No response or songs elemnt in json...";
        throw ParseError( UnknownParseError, QLatin1String( "Invalid json response" ) );
    }

    SongList songs;
    QVariantList songsV = res[ QLatin1String( "response" ) ].toMap()[ QLatin1String( "songs" ) ].toList();

    foreach( const QVariant& s, songsV ) {
        QVariantMap sM = s.toMap();
        Echonest::Song song;

        if( sM.contains( QLatin1String( "title" ) ) )
            song.setTitle( sM[ QLatin1String( "title" ) ].toString() );
        if( sM.contains( QLatin1String( "artist_id" ) ) )
            song.setArtistId( sM[ QLatin1String( "artist_id" ) ].toByteArray() );
        if( sM.contains( QLatin1String( "artist_name" ) ) )
            song.setArtistName( sM[ QLatin1String( "artist_name" ) ].toString() );
        if( sM.contains( QLatin1String( "id" ) ) )
            song.setId( sM[ QLatin1String( "id" ) ].toByteArray() );

        songs.append( song );
    }

    reply->deleteLater();
    return songs;
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
    if( !newSong.artistLocation().location.isEmpty() )
        setArtistLocation( newSong.artistLocation() );
    reply->deleteLater();

}

QVector< Echonest::Song > Echonest::Song::parseSearch( QNetworkReply* reply ) throw( ParseError )
{
    Echonest::Parser::checkForErrors( reply );

    QXmlStreamReader xml( reply->readAll() );

    Echonest::Parser::readStatus( xml );
    QVector<Echonest::Song> songs = Echonest::Parser::parseSongList( xml );

    reply->deleteLater();
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
        case Echonest::Song::Start:
            return "start";
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
        case Echonest::Song::MinEnergy:
            return "min_energy";
        case Echonest::Song::MaxEnergy:
            return "max_energy";
        case Echonest::Song::Mode:
            return "mode";
        case Echonest::Song::Key:
            return "key";
        case Echonest::Song::Sort:
            return "sort";
    }
    return QByteArray();
}


QByteArray Echonest::Song::identifyParamToString( Echonest::Song::IdentifyParam param )
{
    switch( param )
    {
        case Echonest::Song::Code:
            return "code";
        case Echonest::Song::IdentifyArtist:
            return "artist";
        case Echonest::Song::IdentifyDuration:
            return "duration";
        case Echonest::Song::IdentifyGenre:
            return "genre";
        case Echonest::Song::IdentifyRelease:
            return "release";
        case Echonest::Song::IdentifyTitle:
            return "title";
    }
    return QByteArray();
}

void Echonest::Song::addQueryInformation(QUrl& url, Echonest::SongInformation information)
{
    if( information.flags().testFlag( Echonest::SongInformation::AudioSummaryInformation ) )
        url.addEncodedQueryItem( "bucket", "audio_summary" );
    if( information.flags().testFlag( Echonest::SongInformation::Tracks ) )
        url.addEncodedQueryItem( "bucket", "tracks" );
    if( information.flags().testFlag( Echonest::SongInformation::Hotttnesss ) )
        url.addEncodedQueryItem( "bucket", "song_hotttnesss" );
    if( information.flags().testFlag( Echonest::SongInformation::ArtistHotttnesss ) )
        url.addEncodedQueryItem( "bucket", "artist_hotttnesss" );
    if( information.flags().testFlag( Echonest::SongInformation::ArtistFamiliarity ) )
        url.addEncodedQueryItem( "bucket", "artist_familiarity" );
    if( information.flags().testFlag( Echonest::SongInformation::ArtistLocation ) )
        url.addEncodedQueryItem( "bucket", "artist_location" );

    if( !information.idSpaces().isEmpty() ) {
        foreach( const QString& idSpace, information.idSpaces() )
            url.addEncodedQueryItem( "bucket", "id:" + idSpace.toUtf8() );
    }
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

