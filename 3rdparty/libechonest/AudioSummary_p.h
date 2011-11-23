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

#include "Util.h"

#include <QSharedData>
#include <QString>
#include <QUrl>
#include <QVector>

class AudioSummaryData : public QSharedData
{
public:
    AudioSummaryData() : key( -1 ), tempo( -1 ), mode( -1 ), time_signature( -1 ), duration( -1 ), loudness( -1 ), samplerate( -1 ), danceability( -1 ), energy( -1 ),
                         analysis_time( -1 ), status( -1 ), timestamp( -1 ), end_of_fade_in( -1 ), key_confidence( -1 ), mode_confidence( -1 ), num_samples( -1 ),
                         start_of_fade_out( -1 ), tempo_confidence( -1 ), time_signature_confidence( -1 ) {}
    AudioSummaryData(const AudioSummaryData& other) : QSharedData( other )
    {
        key = other.key;
        tempo = other.tempo;
        mode = other.mode;
        time_signature = other.time_signature;
        duration = other.duration;
        loudness = other.loudness;
        samplerate = other.samplerate;
        danceability = other.danceability;
        energy = other.energy;
        
        analysis_url = other.analysis_url;
        
        analysis_time = other.analysis_time;
        analyzer_version = other.analyzer_version;
        detailed_status = other.detailed_status;
        status = other.status;
        timestamp = other.timestamp;
        
        end_of_fade_in = other.end_of_fade_in;
        key_confidence = other.key_confidence;
        loudness = other.loudness;
        mode_confidence = other.mode_confidence;
        num_samples = other.num_samples;
        sample_md5 = other.sample_md5;
        start_of_fade_out = other.start_of_fade_out;
        tempo_confidence = other.tempo_confidence;
        time_signature = other.time_signature;
        time_signature_confidence = other.time_signature_confidence;
        
        bars = other.bars;
        beats = other.beats;
        sections = other.sections;
        tatums = other.tatums;
        segments = other.segments;
        
    }
    
    // basic data that always exists in an Audio Summary object
    int key;
    qreal tempo;
    int mode;
    int time_signature;
    qreal duration;
    qreal loudness;
    int samplerate;
    qreal danceability;
    qreal energy;
    
    QUrl analysis_url; // used to fetch the following pieces of data
    
    // meta section
    qreal analysis_time;
    QString analyzer_version;
    QString detailed_status;
    int status;
    qreal timestamp;
    
    // track section
    qreal end_of_fade_in;
    qreal key_confidence;
    qreal mode_confidence;
    qint64 num_samples;
    QString sample_md5;
    qreal start_of_fade_out;
    qreal tempo_confidence;
    qreal time_signature_confidence;
    
    Echonest::BarList bars;
    Echonest::BeatList beats;
    Echonest::SectionList sections;
    Echonest::TatumList tatums;
    Echonest::SegmentList segments;
    
};
