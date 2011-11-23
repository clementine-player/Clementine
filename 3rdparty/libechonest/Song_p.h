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

#ifndef ECHONEST_SONG_P_H
#define ECHONEST_SONG_P_H

#include "Track.h"
#include "AudioSummary.h"

#include <QSharedData>
#include <QString>
#include <QVector>


class SongData : public QSharedData
{
public:
    SongData() : hotttnesss( -1 ), artistHotttnesss( -1 ), artistFamiliarity( -1 ) { artistLocation.latitude = -1; artistLocation.longitude = -1; }
   
    SongData(const SongData& other) : QSharedData( other )
    {
        id = other.id;
        title = other.title;
        artistName = other.artistName;
        artistId = other.artistId;
        
        audioSummary = other.audioSummary;
        tracks = other.tracks;
        hotttnesss = other.hotttnesss;
        artistHotttnesss = other.artistHotttnesss;
        artistFamiliarity = other.artistFamiliarity;
        artistLocation = other.artistLocation;
    }
    
    ~SongData() {}
    
    QByteArray id;
    QString title;
    QString artistName;
    QByteArray artistId;
    QString release;
    
    // The rest are optional that require manual fetching to populate
    Echonest::AudioSummary audioSummary;
    QVector<Echonest::Track> tracks;
    qreal hotttnesss;
    qreal artistHotttnesss;
    qreal artistFamiliarity;
    Echonest::ArtistLocation artistLocation;
    
    
};

#endif
