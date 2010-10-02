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

#include <QSharedData>
#include <QString>

class TrackData : public QSharedData
{
public:
    TrackData() {}
    
    TrackData(const TrackData& other)
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
    
    Echonest::AudioSummary audio_summary;
    
};

#endif
