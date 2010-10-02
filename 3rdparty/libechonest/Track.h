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


#ifndef ECHONEST_TRACK_H
#define ECHONEST_TRACK_H

#include "AudioSummary.h"
#include "echonest_export.h"
#include "Util.h"

#include <QObject>
#include <QString>
#include <QDebug>
#include <QSharedData>
#include <QUrl>
#include "Config.h"

class QNetworkReply;
class TrackData;

namespace Echonest 
{
  
/**
 * Upload-based Echo Nest Track API. If you want to search The Echo Nest for songs, use the Song API. 
 *  If you want to upload your own files and retrieve the acoustic information about them, use this Track
 *   class. You can also fetch acoustic information from a track if you have the Track ID or MD5 of the file.
 * 
 * A Track encapsulates the audio analysis from The Echo Nest.
 * 
 * This class is implicitly shared.
 * 
 */
class ECHONEST_EXPORT Track
{
public:
 
  Track();
  Track( const Track& other );
  Track& operator=( const Track& track );
  ~Track();
  
  /**
   * The track's artist.
   */
  QString artist() const;
  void setArtist( const QString& artist );
  
  /**
   * The track's title.
   */
  QString title() const;
  void setTitle( const QString& title );
  
  /**
   * The Echo Nest artist ID for this track.
   */
  QByteArray id() const;
  void setId( const QByteArray& id );
  
  /**
   * The MD5 hash of the track.
   */
  QByteArray md5() const;
  void setMD5( const QByteArray& md5 );
  
  /**
   * The album name of this track.
   */
  QString release() const;
  void setRelease( const QString& release );
    
  /**
   * The MD5 hashsum of the audio data.
   */
  QByteArray audioMD5() const;
  void setAudioMD5( const QByteArray& md5 );
  
  /**
   * The analyzer version that was used in this track's analysis.
   */
  QString analyzerVersion() const;
  void setAnalyzerVersion( const QString& analyzerVersion );
  
  /**
   * The samplerate of the track
   */
  int samplerate() const;
  void setSamplerate( int samplerate );
  
  /**
   * The bitrate of the track
   */
  int bitrate() const;  
  void setBitrate( int );
    
  /**
   * The analysis status
   */
  Analysis::AnalysisStatus status() const;  
  void setStatus( Analysis::AnalysisStatus );
  
  /**
   * The full audio summary of the track. This contains information about the track's bars,
   *  beats, sections, and detailed segment information as well as more metadata about the song's
   *  acoustic properties.
   * 
   *  Information about how to interpret the results of the audio summary can be found here:
   *    http://developer.echonest.com/docs/v4/_static/AnalyzeDocumentation_2.2.pdf
   */
  AudioSummary audioSummary() const;
  void setAudioSummary( const AudioSummary& summary );
  
    /**
   * Get a track object from the md5 hash of a song's contents.
   * 
   * Call parseProfile() to get the track itself once the 
   *   QNetworkReply() emits the finished() signal.
   */
  static QNetworkReply* profileFromMD5( const QByteArray& md5 );
  
  /**
   * Get a track object from an Echo Nest track id.
   * 
   * Call parseProfile() to get the track itself once the 
   *   QNetworkReply() emits the finished() signal.
   */
  static QNetworkReply* profileFromTrackId( const QByteArray& id );
  
  /**
   * Upload a track to The Echo Nest for analysis. The file can either be
   *  a local filetype and include the file data as a parameter, or a url to a file on the internet.
   * 
   * When the QNetworkReply emits its finished() signal, you can call parseProfile()
   *  to get the resulting Track object. Be sure to check the status of the new track,
   *  as it might be 'pending', which means it is still being analyzed and must be asked 
   *  for again later.
   * 
   * Note that in the case of uploading a local file, the data QByteArray must stay in scope for the
   *  whole completion of the upload operation.
   */
  static QNetworkReply* uploadLocalFile( const QUrl& localFile, const QByteArray& data, bool waitForResult = true );
  static QNetworkReply* uploadURL( const QUrl& remoteURL, bool waitForResult = true );
  
  /**
   * Analyze a previously uploaded track with the current version of the analyzer. 
   * It can be referenced by either track ID or file md5.
   */
  static QNetworkReply* analyzeTrackId( const QByteArray& id, bool wait = true );
  static QNetworkReply* analyzeTrackMD5( const QByteArray& id, bool wait = true );
  
  /**
   * Parse the result of a track request, and turn it into a
   *  Track object.
   * 
   * Call this function after the QNetworkReply* object returned
   *  from the parse*, upload*, and analyze* emits its finished() signal
   */
  static Track parseProfile( QNetworkReply* ) throw( ParseError );
    
private:
    
    QSharedDataPointer<TrackData> d;
};

ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::Track& track);


} // namespace

#endif
