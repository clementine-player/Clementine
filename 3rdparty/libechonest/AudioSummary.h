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


#ifndef ECHONEST_AUDIOSUMMARY_H
#define ECHONEST_AUDIOSUMMARY_H

#include "echonest_export.h"

#include <QSharedData>
#include <QDebug>
#include "Util.h"
#include "Util.h"
#include "Config.h"

class QNetworkReply;
class QNetworkReply;
class AudioSummaryData;

namespace Echonest{

    /**
     * This encapsulates the audio summary of an Echo Nest track or song.
     * 
     * It has two batches of data: the more generic acoustic information about the
     *  song is always populated, and additional detailed information about the song
     *  such as bars, beats, segments, tatus, and sections, can be fetched as well as
     *  an additional step.
     * 
     * This class is implicitly shared.
     */
    class ECHONEST_EXPORT AudioSummary
    {
    public:
        AudioSummary();
        AudioSummary( const AudioSummary& other );
        ~AudioSummary();
        
        AudioSummary& operator=( const AudioSummary& audio );
        
        int key() const;
        void setKey( int key );
        
        /**
         * The track's tempo.
         */
        qreal tempo() const;
        void setTempo( qreal tempo );
        
        /**
         * The track's mode.
         */
        int mode() const;
        void setMode( int mode );
        
        /**
         * The track's time signature, or -1 if it there is one, or 1 if it is 
         *   too complex.
         */
        int timeSignature() const;
        void setTimeSignature( int timeSignature );
        
        /**
         * The duration of the song, in msecs.
         */
        qreal duration() const;
        void setDuration( qreal duration );
        
        /**
         * The loudness of the song, in dB.
         */
        qreal loudness() const;
        void setLoudness( qreal loudness );
        
        /**
         * The danceability of this track, from 0 to 1.
         */
        qreal danceability() const;
        void setDanceability( qreal dance );
        
        /**
         * The energy of this song, from 0 to 1.
         */
        qreal energy() const;
        void setEnergy( qreal energy );
        
        /// The following require additional fetching to read ///
                         /** TODO: implement **/
        
        /**
         * If you wish to use any of the more detailed track analysis data,
         *  use this method to begin the fetch. One the returned QNetworkReply*
         *  has emitted the finished() signal, call parseFullAnalysis.
         */
        QNetworkReply* fetchFullAnalysis() const;
        
        /**
         * Parses the result of a fetchFullAnalysis() call. This contains
         *  information such as mode, fadein/fadeout, confidence metrics, 
         *  and the division of the song into bars, beats, sections, and segments.
         */
        void parseFullAnalysis( QNetworkReply* reply ) throw( ParseError );
        
        /// The following methods *ALL REQUIRE THAT parseFullAnalysis be called first*
        
        /**
         * How long it took to analyze this track.
         */
        qreal analysisTime() const;
        void setAnalysisTime( qreal time );
        
        /**
         * The version of the analyzer used on this track.
         */
        QString analyzerVersion() const;
        void setAnalyzerVersion( QString version );
        
        /**
         * Detailed status information about the analysis
         */
        QString detailedStatus() const;
        void setDetailedStatus( const QString& status );
        
        /**
         * The status code of the analysis
         */
        int analysisStatus() const;
        void setAnalysisStatus( int status );
        
        /**
         * The timestamp of the analysis.
         */
        qreal timestamp() const;
        void setTimestamp( qreal timestamp );
        
        /** ECHONEST_EXPORT 
         * The sample rate of the track.
         */
        qreal sampleRate() const;
        void setSampleRate( qreal sampleRate );
        
        /**
         * The end of the track's fade in in msec.
         */
        qreal endOfFadeIn() const;
        void setEndOfFadeIn( qreal time );
        
        /**
         * The start of the fade out in msec.
         */
        qreal startOfFadeOut() const;
        void setStartOfFadeOut( qreal time );
        
        /**
         * The confidence of the key item.
         */
        qreal keyConfidence() const;
        void setKeyConfidence( qreal confidence );
        
        /**
         * The confidence of the mode item.
         */
        qreal modeConfidence() const;
        void setModeConfidence( qreal confidence );
        
        /**
         * The confidence of the tempo item.
         */
        qreal tempoConfidence() const;
        void setTempoConfidence( qreal confidence );
        
        /**
         * The confidence of the time signature item.
         */
        qreal timeSignatureConfidence() const;
        void setTimeSignatureConfidence( qreal confidence );
        
        /**
         * The number of samples in this track.
         */
        qint64 numSamples() const;
        void setNumSamples( qint64 num );
        
        /**
         * The MD5 of the sample.
         */
        QString sampleMD5() const;
        void setSampleMD5( const QString& md5 );
        
        /**
         * List of bars that are in the track.
         */
        BarList bars() const;
        void setBars( const BarList& bars );
        
        /**
         * List of beats in the track.
         */
        BeatList beats() const;
        void setBeats( const BeatList& beats );
        
        /**
         * List of sections in the track.
         */
        SectionList sections() const;
        void setSections( const SectionList& sections );
        
        /**
         * List of tatums in the track
         */
        TatumList tatums() const;
        void setTatums( const TatumList& tatums );
        
        /**
         * List of segments in the track with associated acoustic data.
         */
        SegmentList segments() const;
        void setSegments( const SegmentList& segments );
        
        void setAnalysisUrl( const QUrl& analysisUrl );
        
    private:        
        QSharedDataPointer<AudioSummaryData> d;
    };
    
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::AudioSummary& summary);
    
    
} // namespace
#endif
