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

#include "Track.h"

#include "Track_p.h"
#include <QNetworkReply>
#include "Config.h"
#include "Song.h"
#include <qfileinfo.h>
#include "Parsing_p.h"

Echonest::Track::Track()
  :  d( new TrackData )
{
}

Echonest::Track::Track(const Echonest::Track& other)
    : d( other.d )
{}

Echonest::Track::Track(const QByteArray& id)
:  d( new TrackData )
{
    d->id = id;
}


Echonest::Track::~Track()
{
}


Echonest::Track& Echonest::Track::operator=(const Echonest::Track& track)
{
    d = track.d;
    return *this;
}


QString Echonest::Track::artist() const
{
  return d->artist;
}

void Echonest::Track::setArtist(const QString& artist)
{
    d->artist = artist;
}


QString Echonest::Track::title() const
{
  return d->title;
}

void Echonest::Track::setTitle(const QString& title)
{
    d->title = title;
}

QByteArray Echonest::Track::id() const
{
    return d->id;
}

void Echonest::Track::setId(const QByteArray& id)
{
    d->id = id;
}

QByteArray Echonest::Track::md5() const
{
    return d->md5;
}

void Echonest::Track::setMD5(const QByteArray& md5)
{
    d->md5 = md5;
}


QString Echonest::Track::release() const
{
    return d->release;
}

void Echonest::Track::setRelease(const QString& release)
{
    d->release = release;
}

QString Echonest::Track::analyzerVersion() const
{
    return d->analyzer_version;
}

void Echonest::Track::setAnalyzerVersion(const QString& analyzerVersion)
{
    d->analyzer_version = analyzerVersion;
}

int Echonest::Track::bitrate() const
{
    return d->bitrate;
}

void Echonest::Track::setBitrate(int bitrate)
{
    d->bitrate = bitrate;
}

int Echonest::Track::samplerate() const
{
    return d->samplerate;
}

void Echonest::Track::setSamplerate(int samplerate)
{
    d->samplerate = samplerate;
}

QByteArray Echonest::Track::audioMD5() const
{
    return d->audio_md5;
}

void Echonest::Track::setAudioMD5(const QByteArray& md5)
{
    d->audio_md5 = md5;
}

Echonest::Analysis::AnalysisStatus Echonest::Track::status() const
{
    return Echonest::statusToEnum( d->status );
}

void Echonest::Track::setStatus( Echonest::Analysis::AnalysisStatus status )
{
    d->status = Echonest::statusToString( status );
}

Echonest::AudioSummary Echonest::Track::audioSummary() const
{
    return d->audio_summary;
}

void Echonest::Track::setAudioSummary( const Echonest::AudioSummary& summary )
{
    d->audio_summary = summary;
}

QString Echonest::Track::catalog() const
{
    return d->catalog;
}

void Echonest::Track::setCatalog(const QString& catalog)
{
    d->catalog = catalog;
}

QByteArray Echonest::Track::foreignId() const
{
    return d->foreign_id;
}

void Echonest::Track::setForeignId(const QByteArray& id)
{
    d->foreign_id = id;
}

QUrl Echonest::Track::previewUrl() const
{
    return d->preview_url;
}

void Echonest::Track::setPreviewUrl(const QUrl& preview)
{
    d->preview_url = preview;
}

QUrl Echonest::Track::releaseImage() const
{
    return d->release_image;
}

void Echonest::Track::setReleaseImage(const QUrl& imgUrl)
{
    d->release_image = imgUrl;
}

Echonest::Song Echonest::Track::song() const
{
    return d->song;
}

void Echonest::Track::setSong(const Echonest::Song& song)
{
    d->song = song;
}

QNetworkReply* Echonest::Track::profileFromTrackId( const QByteArray& id )
{
    QUrl url = Echonest::baseGetQuery( "track", "profile" );
    url.addEncodedQueryItem( "id", id );
    url.addEncodedQueryItem( "bucket", "audio_summary" );


    qDebug() << "Creating profileFromTrackId URL" << url;
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Track::profileFromMD5( const QByteArray& md5 )
{
    QUrl url = Echonest::baseGetQuery( "track", "profile" );
    url.addEncodedQueryItem( "md5", md5 );
    url.addEncodedQueryItem( "bucket", "audio_summary" );

    qDebug() << "Creating profileFromMD5 URL" << url;
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Track::uploadLocalFile( const QUrl& localFile, const QByteArray& data, bool waitForResult )
{
    QUrl url = Echonest::baseGetQuery( "track", "upload" );
    QFileInfo info( localFile.path() );
    url.addQueryItem( QLatin1String( "filetype" ), info.suffix() );
    url.addEncodedQueryItem( "bucket", "audio_summary" );
    url.addEncodedQueryItem( "wait", ( waitForResult ? "true" : "false" ) );
    QNetworkRequest request( url );

    request.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/octet-stream" ) );
//     qDebug() << "Uploading local file to" << url;
    return Echonest::Config::instance()->nam()->post( request, data );
}

QNetworkReply* Echonest::Track::uploadURL( const QUrl& remoteURL, bool waitForResult )
{
    QUrl url = Echonest::baseGetQuery( "track", "upload" );
    url.addEncodedQueryItem( "url", remoteURL.toEncoded() );
    url.addEncodedQueryItem( "bucket", "audio_summary" );
    url.addEncodedQueryItem( "wait", ( waitForResult ? "true" : "false" ) );

    qDebug() << "Uploading URL:" << url;
    QNetworkRequest req( url );
    req.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/x-www-form-urlencoded" ) );
    return Echonest::Config::instance()->nam()->post( req, QByteArray() );
}

QNetworkReply* Echonest::Track::analyzeTrackId( const QByteArray& id, bool wait )
{
    QUrl url = Echonest::baseGetQuery( "track", "analyze" );
    url.addEncodedQueryItem( "id", id );
    url.addEncodedQueryItem( "bucket", "audio_summary" );
    url.addEncodedQueryItem( "wait", ( wait ? "true" : "false" ) );

    qDebug() << "Creating analyzeTrackId URL" << url;
    return Echonest::doPost( url );
//     return Echonest::Config::instance()->nam()->post( QNetworkRequest( url ), QByteArray() );
}

QNetworkReply* Echonest::Track::analyzeTrackMD5( const QByteArray& md5, bool wait )
{
    QUrl url = Echonest::baseGetQuery( "track", "analyze" );
    url.addEncodedQueryItem( "md5", md5 );
    url.addEncodedQueryItem( "bucket", "audio_summary" );
    url.addEncodedQueryItem( "wait", ( wait ? "true" : "false" ) );

    qDebug() << "Creating analyzeTrackMD5 URL" << url;
    return Echonest::doPost( url );
//     return Echonest::Config::instance()->nam()->post( QNetworkRequest( url ), QByteArray() );
}

Echonest::Track Echonest::Track::parseProfile( QNetworkReply* finishedReply ) throw( Echonest::ParseError )
{
    QByteArray data = finishedReply->readAll();
    qDebug() << data;
    Echonest::Parser::checkForErrors( finishedReply );

    QXmlStreamReader xml( data );

    Echonest::Parser::readStatus( xml );
    Echonest::Track track = Echonest::Parser::parseTrack( xml );

    finishedReply->deleteLater();
    return track;
}



QDebug Echonest::operator<<(QDebug d, const Echonest::Track& track)
{
    d << QString::fromLatin1( "Track(%1, %2, %3, %4)" ).arg( QLatin1String( track.id() ) ).arg( track.title() ).arg( track.artist() ).arg( track.release() );
    return d.maybeSpace();
}

