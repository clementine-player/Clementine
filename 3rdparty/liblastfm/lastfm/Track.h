/*
   Copyright 2009-2010 Last.fm Ltd.
      - Primarily authored by Max Howell, Jono Cole, Doug Mansell and Michael Coffey

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LASTFM_TRACK_H
#define LASTFM_TRACK_H

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QPointer>

#include "Album.h"

namespace lastfm {

class TrackData;

class LASTFM_DLLEXPORT TrackContext
{
public:
    enum Type
    {
        UnknownType,
        User,
        Friend,
        Neighbour,
        Artist
    };

    TrackContext();
    TrackContext( const QString& type, const QList<QString>& values );
    TrackContext( const TrackContext& that );
    ~TrackContext();

    Type type() const;
    QList<QString> values() const;
    TrackContext& operator=( const TrackContext& that );

private:
    class TrackContextPrivate * const d;
};


/** Our track type. It's quite good, you may want to use it as your track type
  * in general. It is explicitly shared. Which means when you make a copy, they
  * both point to the same data still. This is like Qt's implicitly shared
  * classes, eg. QString, however if you mod a copy of a QString, the copy
  * detaches first, so then you have two copies. Our Track object doesn't
  * detach, which is very handy for our usage in the client, but perhaps not
  * what you want. If you need a deep copy for eg. work in a thread, call 
  * clone(). */
class LASTFM_DLLEXPORT Track : public AbstractType
{
public:
    enum Source
    {
        // DO NOT UNDER ANY CIRCUMSTANCES CHANGE THE ORDER OR VALUES OF THIS ENUM!
        // you will cause broken settings and b0rked scrobbler cache submissions

        UnknownSource = 0,
        LastFmRadio,
        Player,
        MediaDevice,
        NonPersonalisedBroadcast, // eg Shoutcast, BBC Radio 1, etc.
        PersonalisedRecommendation // eg Pandora, but not Last.fm
    };

    enum LoveStatus
    {
        UnknownLoveStatus = 0,
        Loved,
        Unloved
    };

    enum ScrobbleStatus
    {
        Null = 0,
        Cached,
        Submitted,
        Error
    };

    enum Corrections
    {
        Original = 0,
        Corrected
    };

    enum ScrobbleError
    {
        None = 0,
        FilteredArtistName = 113,
        FilteredTrackName = 114,
        FilteredAlbumName = 115,
        FilteredTimestamp = 116,
        ExceededMaxDailyScrobbles = 118,
        InvalidStreamAuth = 119,
        Invalid = 300
    };

    Track();
    explicit Track( const QDomElement& );
    Track( const Track& that );
    ~Track();

    Track clone() const;

    /** this track and that track point to the same object, so they are the same
      * in fact. This doesn't do a deep data comparison. So even if all the 
      * fields are the same it will return false if they aren't in fact spawned
      * from the same initial Track object */
    bool sameObject( const Track& that );
    bool operator==( const Track& that ) const;
    bool operator!=( const Track& that ) const;
    Track& operator=( const Track& that );

    /**
     * Track's private class emits three signals that may be useful for
     * applications:
     *
     *   loveToggled( bool love )
     *   scrobbleStatusChanged( short scrobbleStatus )
     *   corrected( QString correction )
     *
     * signalProxy() lets applications connect to them.
     * */
    const QObject* signalProxy() const;

    /** only a Track() is null */
    bool isNull() const;

    bool corrected() const;

    Artist artist( Corrections corrected = Original ) const;
    Artist albumArtist( Corrections corrected = Original ) const;
    Album album( Corrections corrected = Original ) const;
    QString title( Corrections corrected = Original ) const;

    uint trackNumber() const;
    uint duration() const; // in seconds
    Mbid mbid() const;
    QUrl url() const;
    QDateTime timestamp() const;
    Source source() const;
    uint fingerprintId() const;
    bool isLoved() const;
    LoveStatus loveStatus() const;
    QUrl imageUrl( ImageSize size, bool square ) const;

    QString durationString() const;
    static QString durationString( int seconds );

    ScrobbleStatus scrobbleStatus() const;
    ScrobbleError scrobbleError() const;
    QString scrobbleErrorText() const;

    /** default separator is an en-dash */
    QString toString() const;
    QString toString( Corrections corrections ) const;
    QString toString( const QChar& separator, Corrections corrections = Original ) const;
    /** the standard representation of this object as an XML node */
    QDomElement toDomElement( class QDomDocument& ) const;

    TrackContext context() const;

    // iTunes tracks might be podcasts or videos
    bool isPodcast() const;
    bool isVideo() const;
    
    QString extra( const QString& key ) const;

    bool operator<( const Track &that ) const;
    
    bool isMp3() const;
    
    operator QVariant() const;
    
//////////// lastfm::Ws
    
    /** See last.fm/api Track section */
    QNetworkReply* share( const QStringList& recipients, const QString& message = "", bool isPublic = true ) const;

    QNetworkReply* getSimilar( int limit = -1 ) const;
    /** The match percentage is returned from last.fm as a 4 significant
      * figure floating point value. So we multply it by 100 to make an
      * integer in the range of 0 to 10,000. This is possible confusing
      * for you, but I felt it best not to lose any precision, and floats
      * aren't much fun. */
    static QMap<int, QPair< QString, QString > > getSimilar( QNetworkReply* );

    /** you can get any QNetworkReply TagList using Tag::list( QNetworkReply* ) */
    QNetworkReply* getTags() const; // for the logged in user
    QNetworkReply* getTopTags() const;
    QNetworkReply* getTopFans() const;

    /** method should be a method name of reciever that takes a QByteArray
    If that fails it will try invoking method with no arguments.
    */
    void getInfo( QObject* receiver, const char * method, const QString& username = "" ) const;
    QNetworkReply* getBuyLinks( const QString& country ) const;

    static QNetworkReply* playlinks( const QList<Track>& tracks );

    /** you can only add 10 tags, we submit everything you give us, but the
      * docs state 10 only. Will return 0 if the list is empty. */
    QNetworkReply* addTags( const QStringList& ) const;
    /** will return 0 if the string is "" */
    QNetworkReply* removeTag( const QString& ) const;

    /** scrobble the track */
    QNetworkReply* updateNowPlaying() const;
    QNetworkReply* updateNowPlaying( int duration ) const;
    QNetworkReply* removeNowPlaying() const;
    QNetworkReply* scrobble() const;
    static QNetworkReply* scrobble(const QList<lastfm::Track>& tracks);

    /** the url for this track's page at last.fm */
    QUrl www() const;

protected:
    QExplicitlySharedDataPointer<TrackData> d;
    QMap<QString, QString> params( const QString& method, bool use_mbid = false ) const;
};



/** This class allows you to change Track objects, it is easy to use:
  * MutableTrack( some_track_object ).setTitle( "Arse" );
  *
  * We have a separate MutableTrack class because in our usage, tracks
  * only get mutated once, and then after that, very rarely. This pattern
  * encourages such usage, which is generally sensible. You can feel more
  * comfortable that the data hasn't accidently changed behind your back.
  */
class LASTFM_DLLEXPORT MutableTrack : public Track
{
public:
    MutableTrack();

    /** NOTE that passing a Track() to this ctor will automatically make it non
      * null. Which may not be what you want. So be careful
      * Rationale: this is the most maintainable way to do it 
      */
    MutableTrack( const Track& that );

    void setFromLfm( const XmlQuery& lfm );
    void setImageUrl( ImageSize size, const QString& url );
    
    void setArtist( QString artist );
    void setAlbumArtist( QString albumArtist );
    void setAlbum( QString album );
    void setTitle( QString title );
    void setCorrections( QString title, QString album, QString artist, QString albumArtist );
    void setTrackNumber( uint n );
    void setDuration( uint duration );
    void setUrl( QUrl url );
    void setSource( Source s );
    void setLoved( bool loved );
    
    void setMbid( Mbid id );
    void setFingerprintId( uint id );

    void setScrobbleStatus( ScrobbleStatus scrobbleStatus );
    void setScrobbleError( ScrobbleError scrobbleError );
    void setScrobbleErrorText( const QString& scrobbleErrorText );
    
    void love();
    void unlove();
    QNetworkReply* ban();
    
    void stamp();

    void setExtra( const QString& key, const QString& value );
    void removeExtra( QString key );
    void setTimeStamp( const QDateTime& dt );

    void setContext( TrackContext context );

    // iTunes tracks might be podcasts or videos
    void setPodcast( bool podcast );
    void setVideo( bool video );

};


} //namespace lastfm


LASTFM_DLLEXPORT QDebug operator<<( QDebug d, const lastfm::Track& t );

Q_DECLARE_METATYPE( lastfm::Track )

#endif //LASTFM_TRACK_H
