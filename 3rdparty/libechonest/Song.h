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
#include "TypeInformation.h"

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
class Catalog;
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

  enum SearchParam {
      Title,
      Artist,
      Combined,
      Description,
      ArtistId,
      Start,
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
      MaxEnergy,
      MinEnergy,
      Mode,
      Key,
      Sort
  };
  typedef QPair< Echonest::Song::SearchParam, QVariant > SearchParamData;
  typedef QVector< SearchParamData > SearchParams;

  enum IdentifyParam {
      Code,
      IdentifyArtist,
      IdentifyTitle,
      IdentifyRelease,
      IdentifyDuration,
      IdentifyGenre
  };
  typedef QPair< Echonest::Song::IdentifyParam, QVariant > IdentifyParamData;
  typedef QVector< IdentifyParamData > IdentifyParams;

  Song();
  Song( const QByteArray& id, const QString& title, const QByteArray& artistId, const QString& artistName );
  Song( const QByteArray& id );
  Song( const Song& other );
  Song& operator=(const Song& song);
  virtual ~Song();

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

  QString release() const;
  void setRelease( const QString& release );

  /**
   * The following require fetching from The Echo Nest, so call
   *    fetchInformation() with the type of data you want first.
   *
   * If you ask for this information before calling parseInformation()
   *  with the respective data, the result is undefined.
   */

  /**
   * The full audio summary and analysis of this song.
   *
   * NOTE: This will return a copy of the AudioSummary object, which
   *       is implicitly shared. If you make modifications to the returned
   *       summary, for example by calling parseFullAnalysis(), it will detach
   *       and you will have to call setAudioSummary() to save the changes back
   *       to this Song object.
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
  ArtistLocation artistLocation() const;
  void setArtistLocation( const ArtistLocation& artistLocation );

  /**
   *  This fetches the data from The Echo Nest for the requested data, so it
   *   returns a QNetworkReply*. When the finished() signal is emitted
   *   from the QNetworkReply object call parseInformation() to save the
   *   data back to this Song object.
   *
   */
  QNetworkReply* fetchInformation( SongInformation information = SongInformation() ) const;

  /**
   * Search for a song from The Echo Nest with the given search parameters. See
   *  http://developer.echonest.com/docs/v4/song.html#search for a description of the
   *  parameters and data types.
   *
   * The result will contain the requested information from the SongInformation flags, and
   *  can be extracted in the parseSearch() function.
   *
   */
  static QNetworkReply* search( const SearchParams& params, SongInformation information = SongInformation()  );

  /**
   * Identify a song from a given Echo Nest fingerprint hash code.
   * NOTE: SongInformation is currently not parsed yet.
   *
   */
  static QNetworkReply* identify( const IdentifyParams& params, const SongInformation& information = SongInformation() );

  /**
   * Identify a song from the Echoprint hash code, this time using the output of the 'echoprint-codegen' command-line
   * tool
   */
//   static QNetworkReply* identify( const QByteArray& jsonData );

  /**
   * Parses the reply of the identify call and returns a list of songs found.
   *
   */
  static QVector< Song > parseIdentify( QNetworkReply* ) throw( ParseError );

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

  QString toString() const;

  friend class DynamicPlaylist;
  friend class Catalog; // for access to searchParamToString
private:
    static QByteArray searchParamToString( SearchParam param );
    static QByteArray identifyParamToString( IdentifyParam param );
    static void addQueryInformation( QUrl& url, SongInformation information );

  QSharedDataPointer<SongData> d;
};

typedef QVector< Song > SongList;

ECHONEST_EXPORT QDebug operator<<(QDebug d, const Song &song);

} // namespace

Q_DECLARE_METATYPE( Echonest::Song )

#endif
