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

#ifndef ECHONEST_TRACK_P_H
#define ECHONEST_TRACK_P_H

#include "AudioSummary.h"
#include "Config.h"
#include "Song.h"


#include <QSharedData>
#include <QString>
#include <QNetworkReply>

namespace  Echonest {
    inline QNetworkReply* doPost(const QUrl& url)
    {
        // UGLY :( Build url, then extract the encded query items, put them in the POST body, and send that to the url minus the encoded params.
        // The final data
        QByteArray data;
        int size = url.encodedQueryItems().size();
        for( int i = 0; i < size; i++ ) {
            const QPair< QByteArray, QByteArray > item = url.encodedQueryItems().at( i );
            data.append( item.first + "=" + item.second + "&" );
        }
        data.truncate( data.size() - 1 ); // remove extra &
        //qDebug() << "Sending data:" << data << "for method:" << url.path();
        // strip the extras
        QUrl url2( url.toString().mid( 0, url.toString().indexOf( QLatin1Char( '?' ) ) ) );
        QNetworkRequest request = QNetworkRequest( url2 );
        request.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/x-www-form-urlencoded" ) );
        return Echonest::Config::instance()->nam()->post( request, data );
    }
}

class TrackData : public QSharedData
{
public:
    TrackData() {}

    TrackData(const TrackData& other) : QSharedData( other )
    {
        analyzer_version = other.analyzer_version;
        artist = other.artist;
        bitrate = other.bitrate;
        id = other.id;
        md5 = other.md5;
        release = other.release;
        samplerate = other.samplerate;
        status = other.status;
        title = other.title;
        catalog = other.catalog;
        foreign_id = other.foreign_id;
        release_image = other.release_image;
        preview_url = other.preview_url;

    }

    QString artist;
    QString analyzer_version;
    int bitrate;
    QByteArray id;
    QByteArray md5;
    QString release;
    QByteArray audio_md5;
    int samplerate;
    QString status;
    QString title;
    // used when fetched as a foreign id in a tracks bucket
    QString catalog;
    QByteArray foreign_id;
    QUrl release_image;
    QUrl preview_url;

    // song tracks have an associated song
    Echonest::Song song;

    Echonest::AudioSummary audio_summary;

};

#endif
