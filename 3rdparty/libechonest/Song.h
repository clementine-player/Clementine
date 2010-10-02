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


#ifndef ECHONEST_SONG_H
#define ECHONEST_SONG_H

#include "echonest_export.h"
#include "Track.h"

#include <QSharedData>
#include <QHash>
#include <QVariant>
#include <QDebug>
#include <QVector>
#include "Config.h"

class QNetworkReply;
class SongData;
namespace Echonest{

    class DynamicPlaylist; // forward declare for friend declaration
    class AudioSummary;
    
/**
 * This encapsulates an Echo Nest song---use it if you wish to get information about a song,
 *   search for a song, etc.
 * 
 * This class is implicitly shared.
 */
class ECHONEST_EXPORT Song
{
  
public:
  enum SongInformationFlag {
      AudioSummaryInformation = 0x01,
      Tracks = 0x02,
      Hotttnesss = 0x04,
      ArtistHotttnesss = 0x08,
      ArtistFamiliarity = 0x10,
      ArtistLocation = 0x20
  };
  Q_DECLARE_FLAGS( SongInformation, SongInformationFlag )

  enum SearchParam {
      Title,
      Artist,
      Combined,
      Description,
      ArtistId,
      Results,
      MaxTempo,
      MinTempo,
      MaxDanceability,
      MinDanceability,
      MaxComplexity,
      MinComplexity,
      MaxDuration,
      MinDuration,
      MaxLoudness,
      MinLoudness,
      MaxFamiliarity,
      MinFamiliarity,
      MaxHotttnesss,
      MinHotttnesss,
      MaxLongitude,
      MinLongitude,
      Mode,
      Key,
      Sort
  };
  typedef QPair< Echonest::Song::SearchParam, QVariant > SearchParamData;
  typedef QVector< SearchParamData > SearchParams;
  
  Song();
  Song( const QByteArray& id, const QString& title, const QByteArray& artistId, const QString& artistName );
  Song( const Song& other );
  Song& operator=(const Song& song);
  ~Song();
  
  /**
   * The following pieces of data are present in all Song objects, and do not require
   *   on-demand fetching.
   */
  QByteArray id() const;
  void setId( const QByteArray& id );
  
  QString title() const;
  void setTitle( const QString& title );
  
  QString artistName() const;
  void setArtistName( const QString& artistName );
  
  QByteArray artistId() const;
  void setArtistId( const QByteArray& artistId );
  

  /** 
   * The following require fetching from The Echo Nest, so call
   *    fetchInformation() with the type of data you want first.
   * 
   * If you ask for this information before calling parseInformation()
   *  with the respective data, the result is undefined.
   */
  
  /**
   * The full audio summary and analysis of this song.
   */
   AudioSummary audioSummary() const;
   void setAudioSummary( const AudioSummary& summary );
   
  /**
   * The associated Track objects with acoustic track information
   */
  QVector< Track > tracks() const;
  void setTracks( const QVector< Track >& tracks );
  
  /**
   * The "hotttnesss" metric of this song.
   */
  qreal hotttnesss() const;
  void setHotttnesss( qreal hotttnesss );
  
  /**
   * The "hotttnesss" metric of this song's artist.
   */
  qreal artistHotttnesss() const;
  void setArtistHotttnesss( qreal artistHotttnesss );
  
  /**
   * The familiarity metric of this song's artist.
   */
  qreal artistFamiliarity() const;
  void setArtistFamiliarity( qreal artistFamiliarity );
  
  /**
   * The location of this artist.
   */
  QString artistLocation() const;
  void setArtistLocation( const QString& artistLocation );
  
  /**
   *  This fetch the data from The Echo Nest for the requested data, so it
   *   returns a QNetworkReply*. When the finished() signal is emitted 
   *   from the QNetworkReply object call parseInformation() to save the
   *   data back to this Song object.
   * 
   */
  QNetworkReply* fetchInformation( SongInformation parts ) const;
  
  /**
   * Search for a song from The Echo Nest with the given search parameters. See 
   *  http://developer.echonest.com/docs/v4/song.html#search for a description of the
   *  parameters and data types.
   * 
   * The result will contain the requested information from the SongInformation flags, and
   *  can be extracted in the parseSearch() function.
   * 
   */
  static QNetworkReply* search( const SearchParams& params, SongInformation parts );
  
  /**
   * Parse the result of the fetchInformation() call.
   * For each requested SongInformationFlag in the original request, the respective
   *  data will be saved to this Song object.
   */
  void parseInformation( QNetworkReply* reply ) throw( ParseError );
  
  /**
   * Parse the result of the search() call.
   */
  static QVector<Song> parseSearch( QNetworkReply* reply ) throw( ParseError );
  
  /**
   * Identify a song from a given Echo Nest fingerprint hash code
   * 
   * NOTE this is currently not supported, as the Echo Nest hash code
   *      generator is not currently open source, so I don't care much 
   *      for it.
   *
   * static QNetworkReply* identify(  ) const;
   */
  
  QString toString() const;
  
  friend class DynamicPlaylist;
private:
    static QByteArray searchParamToString( SearchParam param );
    static void addQueryInformation( QUrl& url, SongInformation parts );
    
  QSharedDataPointer<SongData> d;
};

typedef QVector< Song > SongList;

ECHONEST_EXPORT QDebug operator<<(QDebug d, const Song &song);

Q_DECLARE_OPERATORS_FOR_FLAGS(Song::SongInformation)

} // namespace

Q_DECLARE_METATYPE( Echonest::Song::SongInformation )

#endif
