/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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

#include "Track.h"
#include "User.h"
#include "UrlBuilder.h"
#include "XmlQuery.h"
#include "ws.h"

#include <QFileInfo>
#include <QStringList>
#include <QAbstractNetworkCache>
#include <QDebug>


class lastfm::TrackContextPrivate
{
    public:
        TrackContext::Type m_type;
        QList<QString> m_values;
        static TrackContext::Type getType( const QString& typeString );
};

lastfm::TrackContext::Type
lastfm::TrackContextPrivate::getType( const QString& typeString )
{
    lastfm::TrackContext::Type type = lastfm::TrackContext::UnknownType;

    if ( typeString == "artist" )
        type = lastfm::TrackContext::Artist;
    else if ( typeString == "user" )
        type = lastfm::TrackContext::User;
    else if ( typeString == "neighbour" )
        type = lastfm::TrackContext::Neighbour;
    else if ( typeString == "friend" )
        type = lastfm::TrackContext::Friend;

    return type;
}


lastfm::TrackContext::TrackContext()
    :d( new TrackContextPrivate )
{
    d->m_type = UnknownType;
}

lastfm::TrackContext::TrackContext( const QString& type, const QList<QString>& values )
    :d( new TrackContextPrivate )
{
    d->m_values = values;
    d->m_type = d->getType( type );
}

lastfm::TrackContext::TrackContext( const TrackContext& that )
    :d( new TrackContextPrivate( *that.d ) )
{
}

lastfm::TrackContext::~TrackContext()
{
    delete d;
}

lastfm::TrackContext::Type
lastfm::TrackContext::type() const
{
    return d->m_type;
}


QList<QString>
lastfm::TrackContext::values() const
{
    return d->m_values;
}

lastfm::TrackContext&
lastfm::TrackContext::operator=( const TrackContext& that )
{
    d->m_type = that.d->m_type;
    d->m_values = that.d->m_values;
    return *this;
}

class TrackObject : public QObject
{
    Q_OBJECT
public:
    TrackObject( lastfm::TrackData& data ) : m_data( data ) {;}

public:
    void forceLoveToggled( bool love );
    void forceScrobbleStatusChanged();
    void forceCorrected( QString correction );

private slots:
    void onLoveFinished();
    void onUnloveFinished();
    void onGotInfo();

signals:
    void loveToggled( bool love );
    void scrobbleStatusChanged( short scrobbleStatus );
    void corrected( QString correction );

private:
    lastfm::TrackData& m_data;
};

class lastfm::TrackData : public QSharedData
{
    friend class TrackObject;

public:
    TrackData();
    ~TrackData();

public:
    lastfm::Artist artist;
    lastfm::Artist albumArtist;
    lastfm::Album album;
    QString title;
    lastfm::Artist correctedArtist;
    lastfm::Artist correctedAlbumArtist;
    lastfm::Album correctedAlbum;
    QString correctedTitle;
    TrackContext context;
    uint trackNumber;
    uint duration;
    short source;
    short rating;
    QString mbid; /// musicbrainz id
    uint fpid;
    QUrl url;
    QDateTime time; /// the time the track was started at
    lastfm::Track::LoveStatus loved;
    QMap<AbstractType::ImageSize, QUrl> m_images;
    short scrobbleStatus;
    short scrobbleError;
    QString scrobbleErrorText;

    //FIXME I hate this, but is used for radio trackauth etc.
    QMap<QString,QString> extras;

    struct Observer
    {
        QNetworkReply* reply;
        QPointer<QObject> receiver;
        const char* method;
    };

    QList<Observer> observers;

    bool null;

    bool podcast;
    bool video;

    TrackObject* trackObject;
};



lastfm::TrackData::TrackData()
             : trackNumber( 0 ),
               duration( 0 ),
               source( Track::UnknownSource ),
               rating( 0 ),
               fpid( -1 ),
               loved( Track::UnknownLoveStatus ),
               scrobbleStatus( Track::Null ),
               scrobbleError( Track::None ),
               null( false ),
               podcast( false ),
               video( false )
{
    trackObject = new TrackObject( *this );
}

lastfm::TrackData::~TrackData()
{
    delete trackObject;
}

lastfm::Track::Track()
    :AbstractType()
{
    d = new TrackData;
    d->null = true;
}

lastfm::Track::Track( const Track& that )
    :AbstractType(), d( that.d )
{
}

lastfm::Track::Track( const QDomElement& e )
    :AbstractType()
{
    d = new TrackData;

    if (e.isNull()) { d->null = true; return; }

    //TODO: not sure of lastfm's xml changed, but <track> nodes have
    // <artist><name>Artist Name</name><mbid>..<url></artist>
    // as children isntead of <artist>Artist Name<artist>
    // we detect both here.
    QDomNode artistName = e.namedItem( "artist" ).namedItem( "name" );
    if( artistName.isNull() ) {
          d->artist = e.namedItem( "artist" ).toElement().text();
    } else {
        d->artist = artistName.toElement().text();

    }

    //TODO: not sure if lastfm xml's changed, or if chart.getTopTracks uses
    //a different format, but the title is stored at
    //<track><name>Title</name>...
    //we detect both here.
    QDomNode trackTitle = e.namedItem( "name" );
    if( trackTitle.isNull() )
        d->title = e.namedItem( "track" ).toElement().text();
    else
        d->title = trackTitle.toElement().text();

    d->albumArtist = e.namedItem( "albumArtist" ).toElement().text();
    d->album =  Album( d->artist, e.namedItem( "album" ).toElement().text() );
    d->correctedArtist = e.namedItem( "correctedArtist" ).toElement().text();
    d->correctedAlbumArtist = e.namedItem( "correctedAlbumArtist" ).toElement().text();
    d->correctedAlbum =  Album( d->correctedArtist, e.namedItem( "correctedAlbum" ).toElement().text() );
    d->correctedTitle = e.namedItem( "correctedTrack" ).toElement().text();
    d->trackNumber = 0;
    d->duration = e.namedItem( "duration" ).toElement().text().toInt();
    d->url = e.namedItem( "url" ).toElement().text();
    d->rating = e.namedItem( "rating" ).toElement().text().toUInt();
    d->source = e.namedItem( "source" ).toElement().text().toInt(); //defaults to 0, or lastfm::Track::UnknownSource
    d->time = QDateTime::fromTime_t( e.namedItem( "timestamp" ).toElement().text().toUInt() );
    d->loved = static_cast<LoveStatus>(e.namedItem( "loved" ).toElement().text().toInt());
    d->scrobbleStatus = e.namedItem( "scrobbleStatus" ).toElement().text().toInt();
    d->scrobbleError = e.namedItem( "scrobbleError" ).toElement().text().toInt();
    d->scrobbleErrorText = e.namedItem( "scrobbleErrorText" ).toElement().text();
    d->podcast = e.namedItem( "podcast" ).toElement().text().toInt();
    d->video = e.namedItem( "video" ).toElement().text().toInt();

    for (QDomElement image = e.firstChildElement("image") ; !image.isNull() ; image = image.nextSiblingElement("image"))
        d->m_images[static_cast<ImageSize>(image.attribute("size").toInt())] = image.text();

    QDomNode artistNode = e.namedItem("artistImages");

    for (QDomElement artistImage = artistNode.firstChildElement("image") ; !artistImage.isNull() ; artistImage = artistImage.nextSiblingElement("image"))
        artist().setImageUrl( static_cast<ImageSize>(artistImage.attribute("size").toInt()), artistImage.text() );

    QDomNode albumNode = e.namedItem("albumImages");

    for (QDomElement albumImage = albumNode.firstChildElement("image") ; !albumImage.isNull() ; albumImage = albumImage.nextSiblingElement("image"))
        album().setImageUrl( static_cast<ImageSize>(albumImage.attribute("size").toInt()), albumImage.text() );


    QDomNodeList nodes = e.namedItem( "extras" ).childNodes();
    for (int i = 0; i < nodes.count(); ++i)
    {
        QDomNode n = nodes.at(i);
        QString key = n.nodeName();
        d->extras[key] = n.toElement().text();
    }
}

void
TrackObject::onLoveFinished()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>(sender()) ) )
    {
        if ( lfm.attribute( "status" ) == "ok")
            m_data.loved = lastfm::Track::Loved;

    }

    emit loveToggled( m_data.loved == lastfm::Track::Loved );
}


void
TrackObject::onUnloveFinished()
{
    lastfm::XmlQuery lfm;

    if ( lfm.parse( static_cast<QNetworkReply*>(sender()) ) )
    {
        if ( lfm.attribute( "status" ) == "ok")
            m_data.loved = lastfm::Track::Unloved;
    }

    emit loveToggled( m_data.loved == lastfm::Track::Loved );
}

void
TrackObject::onGotInfo()
{
    lastfm::TrackData::Observer observer;

    for ( int i = 0 ; i < m_data.observers.count() ; ++i )
    {
        if ( m_data.observers.at( i ).reply == sender() )
        {
            observer = m_data.observers.takeAt( i );
            break;
        }
    }
    
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    const QByteArray data = reply->readAll();

    lastfm::XmlQuery lfm;

    if ( lfm.parse( data ) )
    {
        QString imageUrl = lfm["track"]["image size=small"].text();
        if ( !imageUrl.isEmpty() ) m_data.m_images[lastfm::AbstractType::SmallImage] = imageUrl;
        imageUrl = lfm["track"]["image size=medium"].text();
        if ( !imageUrl.isEmpty() ) m_data.m_images[lastfm::AbstractType::MediumImage] = imageUrl;
        imageUrl = lfm["track"]["image size=large"].text();
        if ( !imageUrl.isEmpty() ) m_data.m_images[lastfm::AbstractType::LargeImage] = imageUrl;
        imageUrl = lfm["track"]["image size=extralarge"].text();
        if ( !imageUrl.isEmpty() ) m_data.m_images[lastfm::AbstractType::ExtraLargeImage] = imageUrl;
        imageUrl = lfm["track"]["image size=mega"].text();
        if ( !imageUrl.isEmpty() ) m_data.m_images[lastfm::AbstractType::MegaImage] = imageUrl;

        if ( lfm["track"]["userloved"].text().length() > 0 )
            m_data.loved = lfm["track"]["userloved"].text() == "0" ? lastfm::Track::Unloved : lastfm::Track::Loved;

        if ( observer.receiver )
            if ( !QMetaObject::invokeMethod( observer.receiver, observer.method, Q_ARG(QByteArray, data) ) )
                QMetaObject::invokeMethod( observer.receiver, observer.method );

        emit loveToggled( m_data.loved == lastfm::Track::Loved );
    }
    else
    {
        if ( observer.receiver )
            if  ( !QMetaObject::invokeMethod( observer.receiver, observer.method, Q_ARG(QByteArray, data) ) )
                QMetaObject::invokeMethod( observer.receiver, observer.method );
    }
}

void
TrackObject::forceLoveToggled( bool love )
{
    emit loveToggled( love );
}

void
TrackObject::forceScrobbleStatusChanged()
{
    emit scrobbleStatusChanged( m_data.scrobbleStatus );
}

void
TrackObject::forceCorrected( QString correction )
{
    emit corrected( correction );
}


lastfm::Track&
lastfm::Track::operator=( const Track& that )
{
    d = that.d;
    return *this;
}

lastfm::Track::~Track()
{
}


lastfm::Track
lastfm::Track::clone() const
{
    Track clone = *this;
    clone.d.detach();
    return clone;
}


QDomElement
lastfm::Track::toDomElement( QDomDocument& xml ) const
{
    QDomElement item = xml.createElement( "track" );
    
    #define makeElement( tagname, getter ) { \
        QString v = getter; \
        if (!v.isEmpty()) \
        { \
            QDomElement e = xml.createElement( tagname ); \
            e.appendChild( xml.createTextNode( v ) ); \
            item.appendChild( e ); \
        } \
    }

    makeElement( "artist", d->artist );
    makeElement( "albumArtist", d->albumArtist );
    makeElement( "album", d->album );
    makeElement( "track", d->title );
    makeElement( "correctedArtist", d->correctedArtist );
    makeElement( "correctedAlbumArtist", d->correctedAlbumArtist );
    makeElement( "correctedAlbum", d->correctedAlbum );
    makeElement( "correctedTrack", d->correctedTitle );
    makeElement( "duration", QString::number( d->duration ) );
    makeElement( "timestamp", QString::number( d->time.toTime_t() ) );
    makeElement( "url", d->url.toString() );
    makeElement( "source", QString::number( d->source ) );
    makeElement( "rating", QString::number(d->rating) );
    makeElement( "fpId", QString::number(d->fpid) );
    makeElement( "mbId", mbid() );
    makeElement( "loved", QString::number( d->loved ) );
    makeElement( "scrobbleStatus", QString::number( scrobbleStatus() ) );
    makeElement( "scrobbleError", QString::number( scrobbleError() ) );
    makeElement( "scrobbleErrorText", scrobbleErrorText() );
    makeElement( "podcast", QString::number( isPodcast() ) );
    makeElement( "video", QString::number( isVideo() ) );

    // put the images urls in the dom
    QMapIterator<ImageSize, QUrl> imageIter( d->m_images );
    while (imageIter.hasNext()) {
        QDomElement e = xml.createElement( "image" );
        e.appendChild( xml.createTextNode( imageIter.next().value().toString() ) );
        e.setAttribute( "size", imageIter.key() );
        item.appendChild( e );
    }

    QDomElement artistElement = xml.createElement( "artistImages" );

    for ( int size = SmallImage ; size <= MegaImage ; ++size )
    {
        QString imageUrl = d->artist.imageUrl( static_cast<ImageSize>(size) ).toString();

        if ( !imageUrl.isEmpty() )
        {
            QDomElement e = xml.createElement( "image" );
            e.appendChild( xml.createTextNode( d->artist.imageUrl( static_cast<ImageSize>(size) ).toString() ) );
            e.setAttribute( "size", size );
            artistElement.appendChild( e );
        }
    }

    if ( artistElement.childNodes().count() != 0 )
        item.appendChild( artistElement );

    QDomElement albumElement = xml.createElement( "albumImages" );

    for ( int size = SmallImage ; size <= MegaImage ; ++size )
    {
        QString imageUrl = d->album.imageUrl( static_cast<ImageSize>(size) ).toString();

        if ( !imageUrl.isEmpty() )
        {
            QDomElement e = xml.createElement( "image" );
            e.appendChild( xml.createTextNode( d->album.imageUrl( static_cast<ImageSize>(size) ).toString() ) );
            e.setAttribute( "size", size );
            albumElement.appendChild( e );
        }
    }

    if ( albumElement.childNodes().count() != 0 )
        item.appendChild( albumElement );

    // add the extras to the dom
    QDomElement extras = xml.createElement( "extras" );
    QMapIterator<QString, QString> extrasIter( d->extras );
    while (extrasIter.hasNext()) {
        QDomElement e = xml.createElement( extrasIter.next().key() );
        e.appendChild( xml.createTextNode( extrasIter.value() ) );
        extras.appendChild( e );
    }
    item.appendChild( extras );

    return item;
}


bool
lastfm::Track::corrected() const
{
    // If any of the corrected string have been set and they are different
    // from the initial strings then this track has been corrected.
    return ( (!d->correctedTitle.isEmpty() && (d->correctedTitle != d->title))
            || (!d->correctedAlbum.toString().isEmpty() && (d->correctedAlbum.toString() != d->album.toString()))
            || (!d->correctedArtist.isNull() && (d->correctedArtist.name() != d->artist.name()))
            || (!d->correctedAlbumArtist.isNull() && (d->correctedAlbumArtist.name() != d->albumArtist.name())));
}

lastfm::Artist
lastfm::Track::artist( Corrections corrected ) const
{
    if ( corrected == Corrected && !d->correctedArtist.name().isEmpty() )
        return d->correctedArtist;

    return d->artist;
}

lastfm::Artist
lastfm::Track::albumArtist( Corrections corrected ) const
{
    if ( corrected == Corrected && !d->correctedAlbumArtist.name().isEmpty() )
        return d->correctedAlbumArtist;

    return d->albumArtist;
}

lastfm::Album
lastfm::Track::album( Corrections corrected ) const
{
    if ( corrected == Corrected && !d->correctedAlbum.title().isEmpty() )
        return d->correctedAlbum;

    return d->album;
}

QString
lastfm::Track::title( Corrections corrected ) const
{
    /** if no title is set, return the musicbrainz unknown identifier
      * in case some part of the GUI tries to display it anyway. Note isNull
      * returns false still. So you should have queried this! */

    if ( corrected == Corrected && !d->correctedTitle.isEmpty() )
        return d->correctedTitle;

    return d->title;
}


QUrl
lastfm::Track::imageUrl( ImageSize size, bool square ) const
{
    if( !square ) return d->m_images.value( size );

    QUrl url = d->m_images.value( size );
    QRegExp re( "/serve/(\\d*)s?/" );
    return QUrl( url.toString().replace( re, "/serve/\\1s/" ));
}


QString
lastfm::Track::toString( const QChar& separator, Corrections corrections ) const
{
    if ( d->artist.name().isEmpty() )
    {
        if ( d->title.isEmpty() )
            return QFileInfo( d->url.path() ).fileName();
        else
            return title( corrections );
    }

    if ( d->title.isEmpty() )
        return artist( corrections );

    return artist( corrections ) + ' ' + separator + ' ' + title( corrections );
}


QString //static
lastfm::Track::durationString( int const duration )
{
    QTime t = QTime().addSecs( duration );
    if (duration < 60*60)
        return t.toString( "m:ss" );
    else
        return t.toString( "hh:mm:ss" );
}


QNetworkReply*
lastfm::Track::share( const QStringList& recipients, const QString& message, bool isPublic ) const
{
    QMap<QString, QString> map = params("share");
    map["recipient"] = recipients.join(",");
    map["public"] = isPublic ? "1" : "0";
    if (message.size()) map["message"] = message;
    return ws::post(map);
}

void
lastfm::MutableTrack::setFromLfm( const XmlQuery& lfm )
{
    QString imageUrl = lfm["track"]["image size=small"].text();
    if ( !imageUrl.isEmpty() ) d->m_images[SmallImage] = imageUrl;
    imageUrl = lfm["track"]["image size=medium"].text();
    if ( !imageUrl.isEmpty() ) d->m_images[MediumImage] = imageUrl;
    imageUrl = lfm["track"]["image size=large"].text();
    if ( !imageUrl.isEmpty() ) d->m_images[LargeImage] = imageUrl;
    imageUrl = lfm["track"]["image size=extralarge"].text();
    if ( !imageUrl.isEmpty() ) d->m_images[ExtraLargeImage] = imageUrl;
    imageUrl = lfm["track"]["image size=mega"].text();
    if ( !imageUrl.isEmpty() ) d->m_images[MegaImage] = imageUrl;

    if ( lfm["track"]["userloved"].text().length() > 0)
        d->loved = lfm["track"]["userloved"].text() == "0" ? Unloved : Loved;

    d->trackObject->forceLoveToggled( d->loved == Loved );
}

void
lastfm::MutableTrack::setImageUrl( ImageSize size, const QString& url )
{
    if ( !url.isEmpty() )
        d->m_images[size] = url;
}


void
lastfm::MutableTrack::love()
{
    QNetworkReply* reply = ws::post(params("love"));
    QObject::connect( reply, SIGNAL(finished()), signalProxy(), SLOT(onLoveFinished()));
}


void
lastfm::MutableTrack::unlove()
{
    QNetworkReply* reply = ws::post(params("unlove"));
    QObject::connect( reply, SIGNAL(finished()), signalProxy(), SLOT(onUnloveFinished()));
}

QNetworkReply*
lastfm::MutableTrack::ban()
{
    d->extras["rating"] = "B";
    return ws::post(params("ban"));
}


QMap<QString, QString>
lastfm::Track::params( const QString& method, bool use_mbid ) const
{
    QMap<QString, QString> map;
    map["method"] = "Track."+method;
    if (d->mbid.size() && use_mbid)
        map["mbid"] = d->mbid;
    else {
        map["artist"] = d->artist;
        map["track"] = d->title;
    }
    return map;
}


QNetworkReply* 
lastfm::Track::getSimilar( int limit ) const
{
    QMap<QString, QString> map = params("getSimilar");
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    map["autocorrect"] = "1";
    return ws::get( map );
}


QMap<int, QPair< QString, QString > > /* static */
lastfm::Track::getSimilar( QNetworkReply* r )
{
    QMap<int, QPair< QString, QString > > tracks;
    try
    {
        XmlQuery lfm;
        
        if ( lfm.parse( r ) )
        {
            foreach (XmlQuery e, lfm.children( "track" ))
            {
                QPair< QString, QString > track;
                track.first = e["name"].text();

                XmlQuery artist = e.children( "artist" ).first();
                track.second = artist["name"].text();

                // convert floating percentage to int in range 0 to 10,000
                int const match = e["match"].text().toFloat() * 100;
                tracks.insertMulti( match, track );
            }
        }
    }
    catch (ws::ParseError& e)
    {
        qWarning() << e.message();
    }
    
    return tracks;
}


QNetworkReply*
lastfm::Track::getTopTags() const
{
    return ws::get( params("getTopTags", true) );
}


QNetworkReply*
lastfm::Track::getTopFans() const
{
    return ws::get( params("getTopFans", true) );
}


QNetworkReply*
lastfm::Track::getTags() const
{
    return ws::get( params("getTags", true) );
}

void
lastfm::Track::getInfo( QObject *receiver, const char *method, const QString &username ) const
{
    QMap<QString, QString> map = params("getInfo", true);
    if (!username.isEmpty()) map["username"] = username;

    // this is so the web services knows whether to use corrections or not
    if (!lastfm::ws::SessionKey.isEmpty()) map["sk"] = lastfm::ws::SessionKey;

    QNetworkReply* reply = ws::get( map );

    TrackData::Observer observer;
    observer.receiver = receiver;
    observer.method = method;
    observer.reply = reply;
    d->observers << observer;

    QObject::connect( reply, SIGNAL(finished()), d->trackObject, SLOT(onGotInfo()));
}


QNetworkReply*
lastfm::Track::getBuyLinks( const QString& country ) const
{
    QMap<QString, QString> map = params( "getBuyLinks", true );
    map["country"] = country;
    return ws::get( map );
}

QNetworkReply*
lastfm::Track::playlinks( const QList<Track>& tracks )
{
    QMap<QString, QString> map;

    map["method"] = "Track.playlinks";

    for ( int i = 0 ; i < tracks.count() ; ++i )
    {
        if ( tracks[i].d->mbid.size())
            map["mbid[" + QString::number( i ) + "]"] = tracks[i].d->mbid;
        else
        {
            map["artist[" + QString::number( i ) + "]"] = tracks[i].d->artist;
            map["track[" + QString::number( i ) + "]"] = tracks[i].d->title;
        }
    }

    return ws::get( map );
}


QNetworkReply*
lastfm::Track::addTags( const QStringList& tags ) const
{
    if (tags.isEmpty())
        return 0;
    QMap<QString, QString> map = params("addTags");
    map["tags"] = tags.join( QChar(',') );
    return ws::post(map);
}


QNetworkReply*
lastfm::Track::removeTag( const QString& tag ) const
{
    if (tag.isEmpty())
        return 0;
    QMap<QString, QString> map = params( "removeTag" );
    map["tag"] = tag;
    return ws::post(map);
}


QNetworkReply*
lastfm::Track::updateNowPlaying() const
{
    return updateNowPlaying(duration());
}

QNetworkReply* 
lastfm::Track::updateNowPlaying( int duration ) const
{
    QMap<QString, QString> map = params("updateNowPlaying");
    map["duration"] = QString::number( duration );
    map["albumArtist"] = d->albumArtist;
    if ( !album().isNull() ) map["album"] = album();
    map["context"] = extra("playerId");

    return ws::post(map);
}

QNetworkReply* 
lastfm::Track::removeNowPlaying() const
{
    QMap<QString, QString> map;
    map["method"] = "track.removeNowPlaying";

    return ws::post(map);
}


QNetworkReply*
lastfm::Track::scrobble() const
{
    QMap<QString, QString> map = params("scrobble");
    map["duration"] = QString::number( d->duration );
    map["timestamp"] = QString::number( d->time.toTime_t() );
    map["context"] = extra("playerId");
    map["albumArtist"] = d->albumArtist;
    if ( !d->album.title().isEmpty() ) map["album"] = d->album.title();
    map["chosenByUser"] = source() == Track::LastFmRadio ? "0" : "1";

    return ws::post(map);
}

QNetworkReply*
lastfm::Track::scrobble(const QList<lastfm::Track>& tracks)
{
    QMap<QString, QString> map;
    map["method"] = "track.scrobble";

    for ( int i(0) ; i < tracks.count() ; ++i )
    {
        map["duration[" + QString::number(i) + "]"] = QString::number( tracks[i].duration() );
        map["timestamp[" + QString::number(i)  + "]"] = QString::number( tracks[i].timestamp().toTime_t() );
        map["track[" + QString::number(i)  + "]"] = tracks[i].title();
        map["context[" + QString::number(i)  + "]"] = tracks[i].extra("playerId");
        if ( !tracks[i].album().isNull() ) map["album[" + QString::number(i)  + "]"] = tracks[i].album();
        map["artist[" + QString::number(i) + "]"] = tracks[i].artist();
        map["albumArtist[" + QString::number(i) + "]"] = tracks[i].albumArtist();
        if ( !tracks[i].mbid().isNull() ) map["mbid[" + QString::number(i)  + "]"] = tracks[i].mbid();
        map["chosenByUser[" + QString::number(i) + "]"] = tracks[i].source() == Track::LastFmRadio ? "0" : "1";
    }

    return ws::post(map);
}


QUrl
lastfm::Track::www() const
{
    return UrlBuilder( "music" ).slash( artist( Corrected ) ).slash( album(  Corrected  ).isNull() ? QString("_") : album( Corrected )).slash( title( Corrected ) ).url();
}


bool
lastfm::Track::isMp3() const
{
    //FIXME really we should check the file header?
    return d->url.scheme() == "file" &&
           d->url.path().endsWith( ".mp3", Qt::CaseInsensitive );
}

bool
lastfm::Track::sameObject( const Track& that )
{
    return (this->d == that.d);
}

bool
lastfm::Track::operator==( const Track& that ) const
{
    return ( title( Corrected ) == that.title( Corrected )
             // if either album is empty, assume they are the same album
             && ( album( Corrected ).title().isEmpty() || that.album( Corrected ).title().isEmpty() || album( Corrected ) == that.album( Corrected ))
             && artist( Corrected ) == that.artist( Corrected ));
}

bool
lastfm::Track::operator!=( const Track& that ) const
{
    return !operator==( that );
}

const QObject*
lastfm::Track::signalProxy() const
{
    return d->trackObject;
}

bool
lastfm::Track::isNull() const
{
    return d->null;
}

uint
lastfm::Track::trackNumber() const
{ return d->trackNumber; }
uint
lastfm::Track::duration() const
{
    // in seconds
    return d->duration;
}

lastfm::Mbid
lastfm::Track::mbid() const
{
    return lastfm::Mbid(d->mbid); }
QUrl
lastfm::Track::url() const
{
    return d->url; }
QDateTime
lastfm::Track::timestamp() const
{
    return d->time;
}

lastfm::Track::Source
lastfm::Track::source() const
{
    return static_cast<Source>(d->source);
}

uint
lastfm::Track::fingerprintId() const
{
    return d->fpid;
}

bool
lastfm::Track::isLoved() const
{
    return d->loved == Loved;
}

lastfm::Track::LoveStatus
lastfm::Track::loveStatus() const
{
    return d->loved;
}


QString
lastfm::Track::durationString() const
{
    return durationString( d->duration );
}


lastfm::Track::ScrobbleStatus
lastfm::Track::scrobbleStatus() const
{
    return static_cast<ScrobbleStatus>(d->scrobbleStatus);
}

lastfm::Track::ScrobbleError
lastfm::Track::scrobbleError() const
{
    return static_cast<ScrobbleError>(d->scrobbleError);
}
QString
lastfm::Track::scrobbleErrorText() const
{
    return d->scrobbleErrorText;
}

/** default separator is an en-dash */
QString
lastfm::Track::toString() const
{
    return toString( Corrected );
}

QString
lastfm::Track::toString( Corrections corrections ) const
{
    return toString( QChar(8211), corrections );
}

lastfm::TrackContext
lastfm::Track::context() const
{
    return d->context;
}

// iTunes tracks might be podcasts or videos
bool
lastfm::Track::isPodcast() const
{
    return d->podcast;
}

bool
lastfm::Track::isVideo() const
{
    return d->video;
}

QString
lastfm::Track::extra( const QString& key ) const
{
    return d->extras[ key ];
}

bool lastfm::Track::operator<( const Track &that ) const
{
    return this->d->time < that.d->time;
}

lastfm::Track::operator QVariant() const
{
    return QVariant::fromValue( *this );
}

void
lastfm::MutableTrack::setCorrections( QString title, QString album, QString artist, QString albumArtist )
{
    d->correctedTitle = title;
    d->correctedArtist = artist;
    d->correctedAlbum = Album( artist, album );
    d->correctedAlbumArtist = albumArtist;

    d->trackObject->forceCorrected( toString() );
}

lastfm::MutableTrack::MutableTrack()
{
    d->null = false;
}


lastfm::MutableTrack::MutableTrack( const Track& that )
    : Track( that )
{
    d->null = false;
}

void
lastfm::MutableTrack::setArtist( QString artist )
{
    d->artist.setName( artist.trimmed() );
    d->album.setArtist( artist.trimmed() );
    d->correctedAlbum.setArtist( artist.trimmed() );
}

void
lastfm::MutableTrack::setAlbumArtist( QString albumArtist )
{
    d->albumArtist.setName( albumArtist.trimmed() );
}

void
lastfm::MutableTrack::setAlbum( QString album )
{
    d->album = Album( d->artist.name(), album.trimmed() );
}

void
lastfm::MutableTrack::setTitle( QString title )
{
    d->title = title.trimmed();
}

void
lastfm::MutableTrack::setTrackNumber( uint n )
{
    d->trackNumber = n;
}

void
lastfm::MutableTrack::setDuration( uint duration )
{
    d->duration = duration;
}

void
lastfm::MutableTrack::setUrl( QUrl url )
{
    d->url = url;
}

void
lastfm::MutableTrack::setSource( Source s )
{
    d->source = s;
}

void
lastfm::MutableTrack::setLoved( bool loved )
{
    d->loved = loved ? Loved : Unloved;
}

void
lastfm::MutableTrack::setMbid( Mbid id )
{
    d->mbid = id;
}

void
lastfm::MutableTrack::setFingerprintId( uint id )
{
    d->fpid = id;
}

void
lastfm::MutableTrack::setScrobbleStatus( ScrobbleStatus scrobbleStatus )
{
    if ( scrobbleStatus != d->scrobbleStatus )
    {
        d->scrobbleStatus = scrobbleStatus;
        d->trackObject->forceScrobbleStatusChanged();
    }
}

void
lastfm::MutableTrack::setScrobbleError( ScrobbleError scrobbleError )
{
    d->scrobbleError = scrobbleError;
}

void
lastfm::MutableTrack::setScrobbleErrorText( const QString& scrobbleErrorText )
{
    d->scrobbleErrorText = scrobbleErrorText;
}

void
lastfm::MutableTrack::stamp()
{
    d->time = QDateTime::currentDateTime();
}

void
lastfm::MutableTrack::setExtra( const QString& key, const QString& value )
{
    d->extras[key] = value;
}

void
lastfm::MutableTrack::removeExtra( QString key )
{
    d->extras.remove( key );
}

void
lastfm::MutableTrack::setTimeStamp( const QDateTime& dt )
{
    d->time = dt;
}

void
lastfm::MutableTrack::setContext( TrackContext context )
{
    d->context = context;
}

// iTunes tracks might be podcasts or videos
void
lastfm::MutableTrack::setPodcast( bool podcast )
{
    d->podcast = podcast;
}
void
lastfm::MutableTrack::setVideo( bool video )
{
    d->video = video;
}

QDebug
operator<<( QDebug d, const lastfm::Track& t )
{
    return !t.isNull()
            ? d << t.toString( '-' ) << t.url()
            : d << "Null Track object";
}

#include "Track.moc"
