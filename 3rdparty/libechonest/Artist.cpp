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

#include "Artist.h"
#include "Artist_p.h"
#include "ArtistTypes.h"
#include "Parsing_p.h"

Echonest::Artist::Artist()
    : d( new ArtistData )
{
}

Echonest::Artist::Artist( const QByteArray& id, const QString& name )
    : d( new ArtistData )
{
    d->id = id;
    d->name = name;
}

Echonest::Artist::Artist(const QString& name)
    : d( new ArtistData )
{
    setName( name );
}

Echonest::Artist::Artist(const Echonest::Artist& other)
    : d( other.d )
{}

Echonest::Artist& Echonest::Artist::operator=(const Echonest::Artist& artist)
{
    d = artist.d;
    return *this;
}

Echonest::Artist::~Artist()
{

}


QByteArray Echonest::Artist::id() const
{
    return d->id;
}

QString Echonest::Artist::name() const
{
    return d->name;
}

void Echonest::Artist::setId(const QByteArray& id)
{
    d->id = id;
}

void Echonest::Artist::setName(const QString& name)
{
    d->name = name;
}

Echonest::AudioList Echonest::Artist::audio() const
{
    return d->audio;
}

void Echonest::Artist::setAudio(const Echonest::AudioList& audio)
{
    d->audio = audio;
}


Echonest::BiographyList Echonest::Artist::biographies() const
{
    return d->biographies;
}

void Echonest::Artist::setBiographies(const Echonest::BiographyList& bios )
{
    d->biographies = bios;
}

Echonest::BlogList Echonest::Artist::blogs() const
{
    return d->blogs;
}

void Echonest::Artist::setBlogs(const Echonest::BlogList& blogs )
{
    d->blogs = blogs;
}

qreal Echonest::Artist::familiarity() const
{
    return d->familiarity;
}

void Echonest::Artist::setFamiliarity(qreal familiar)
{
    d->familiarity = familiar;
}

qreal Echonest::Artist::hotttnesss() const
{
    return d->hotttnesss;
}

void Echonest::Artist::setHotttnesss(qreal hotttnesss)
{
    d->hotttnesss = hotttnesss;
}

Echonest::ArtistImageList Echonest::Artist::images() const
{
    return d->images;
}

void Echonest::Artist::setImages(const Echonest::ArtistImageList& imgs)
{
    d->images = imgs;
}

Echonest::NewsList Echonest::Artist::news() const
{
    return d->news;
}

void Echonest::Artist::setNews(const Echonest::NewsList& news)
{
    d->news = news;
}

Echonest::ReviewList Echonest::Artist::reviews() const
{
    return d->reviews;
}

void Echonest::Artist::setReviews(const Echonest::ReviewList& reviews)
{
    d->reviews = reviews;
}

Echonest::SongList Echonest::Artist::songs() const
{
    return d->songs;
}

void Echonest::Artist::setSongs(const Echonest::SongList& songs)
{
    d->songs = songs;
}

Echonest::TermList Echonest::Artist::terms() const
{
    return d->terms;
}

void Echonest::Artist::setTerms(const Echonest::TermList& terms)
{
    d->terms = terms;
}

QUrl Echonest::Artist::amazonUrl() const
{
    return d->amazon_url;
}

void Echonest::Artist::setVideos(const Echonest::VideoList& videos)
{
    d->videos = videos;
}

void Echonest::Artist::setAmazonUrl(const QUrl& url)
{
    d->amazon_url = url;
}


QUrl Echonest::Artist::aolMusicUrl() const
{
    return d->aolmusic_url;
}

void Echonest::Artist::setAolMusicUrl(const QUrl& url)
{
    d->aolmusic_url = url;
}

QUrl Echonest::Artist::itunesUrl() const
{
    return d->itunes_url;
}

void Echonest::Artist::setItunesUrl( const QUrl& url )
{
    d->itunes_url = url;
}

QUrl Echonest::Artist::lastFmUrl() const
{
    return d->lastfm_url;
}

void Echonest::Artist::setLastFmUrl(const QUrl& url )
{
    d->lastfm_url = url;
}

QUrl Echonest::Artist::myspaceUrl() const
{
    return d->myspace_url;
}

void Echonest::Artist::setMyspaceUrl( const QUrl& url )
{
    d->myspace_url = url;
}

QUrl Echonest::Artist::musicbrainzUrl() const
{
    return d->mb_url;
}

void Echonest::Artist::setMusicbrainzUrl(const QUrl& url)
{
    d->mb_url = url;
}


Echonest::VideoList Echonest::Artist::videos() const
{
    return d->videos;
}


QNetworkReply* Echonest::Artist::fetchAudio(int numResults, int offset) const
{
    QUrl url = setupQuery( "audio", numResults, offset );
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchBiographies(const QString& license, int numResults, int offset) const
{
    QUrl url = setupQuery( "biographies", numResults, offset );
    if( !license.isEmpty() )
        url.addQueryItem( QLatin1String( "license" ), license );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchBlogs( bool highRelevanceOnly, int numResults, int offset ) const
{
    QUrl url = setupQuery( "blogs", numResults, offset );
    if( highRelevanceOnly ) // false is default
        url.addEncodedQueryItem( "high_relevance", "true" );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchFamiliarity() const
{
    QUrl url = setupQuery( "familiarity", 0, -1 );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchHotttnesss(const QString& type) const
{
    QUrl url = setupQuery( "hotttnesss", 0, -1 );
    if( type != QLatin1String( "normal" ) )
        url.addQueryItem( QLatin1String( "type" ), type );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchImages( const QString& license, int numResults, int offset ) const
{
    QUrl url = setupQuery( "images", numResults, offset );
    if( !license.isEmpty() )
        url.addQueryItem( QLatin1String( "license" ), license );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchProfile(Echonest::Artist::ArtistInformation information) const
{
    QUrl url = setupQuery( "profile", 0, -1 );
    addQueryInformation( url, information );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchNews( bool highRelevanceOnly, int numResults, int offset ) const
{
    QUrl url = setupQuery( "news", numResults, offset );
    if( highRelevanceOnly ) // false is default
    url.addEncodedQueryItem( "high_relevance", "true" );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchReviews(int numResults, int offset) const
{
    QUrl url = setupQuery( "reviews", numResults, offset );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchSimilar(const Echonest::Artist::SearchParams& params, Echonest::Artist::ArtistInformation information, int numResults, int offset )
{
    QUrl url = Echonest::baseGetQuery( "artist", "similar" );
    addQueryInformation( url, information );
    
    if( numResults > 0 )
        url.addEncodedQueryItem( "results", QByteArray::number( numResults ) );
    if( offset >= 0 ) 
        url.addEncodedQueryItem( "start", QByteArray::number( offset ) );
        
    Echonest::Artist::SearchParams::const_iterator iter = params.constBegin();
    for( ; iter < params.constEnd(); ++iter )
        url.addQueryItem( QLatin1String( searchParamToString( iter->first ) ), iter->second.toString().replace( QLatin1Char( ' ' ), QLatin1Char( '+' ) ) );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchSongs( Echonest::Artist::ArtistInformation idspace, bool limitToIdSpace, int numResults, int offset  ) const
{
    QUrl url = setupQuery( "songs", numResults, offset );
    addQueryInformation( url, idspace );
    if( limitToIdSpace )
        url.addEncodedQueryItem( "limit", "true" );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchTerms( Echonest::Artist::TermSorting sorting ) const
{
    QUrl url = setupQuery( "terms", 0, -1 );
    if( sorting == Echonest::Artist::Weight )
        url.addEncodedQueryItem( "sort", "weight" );
    else if( sorting == Echonest::Artist::Frequency )
        url.addEncodedQueryItem( "sort", "frequency" );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}


QNetworkReply* Echonest::Artist::fetchUrls() const
{
    QUrl url = setupQuery( "urls", 0, -1 );

    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::fetchVideo(int numResults, int offset) const
{
    QUrl url = setupQuery( "video", numResults, offset );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::search(const Echonest::Artist::SearchParams& params, Echonest::Artist::ArtistInformation information, bool limit)
{
    QUrl url = Echonest::baseGetQuery( "artist", "search" );
    
    Echonest::Artist::SearchParams::const_iterator iter = params.constBegin();
    for( ; iter < params.constEnd(); ++iter )
        url.addQueryItem( QLatin1String( searchParamToString( iter->first ) ), iter->second.toString().replace( QLatin1Char( ' ' ), QLatin1Char( '+' ) ) );
    url.addEncodedQueryItem( "limit", limit ? "true" : "false" );
    addQueryInformation( url, information );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::topHottt(Echonest::Artist::ArtistInformation information, int numResults, int offset, bool limit)
{
    QUrl url = Echonest::baseGetQuery( "artist", "top_hottt" );
    addQueryInformation( url, information );
    
    if( numResults > 0 )
        url.addEncodedQueryItem( "results", QByteArray::number( numResults ) );
    if( offset >= 0 ) 
        url.addEncodedQueryItem( "start", QByteArray::number( offset ) );
    
    url.addEncodedQueryItem( "limit", limit ? "true" : "false" );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

QNetworkReply* Echonest::Artist::topTerms(int numResults)
{
    QUrl url = Echonest::baseGetQuery( "artist", "top_terms" );
    url.addEncodedQueryItem( "results", QByteArray::number( numResults ) );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );
}

int Echonest::Artist::parseProfile( QNetworkReply* reply ) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    
    QXmlStreamReader xml( reply->readAll() );
    
    Echonest::Parser::readStatus( xml );
    
    int numResults = Echonest::Parser::parseArtistInfoOrProfile( xml, *this );
    
    return numResults;
}

Echonest::Artists Echonest::Artist::parseSearch( QNetworkReply* reply ) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    
    QXmlStreamReader xml( reply->readAll() );
    
    Echonest::Parser::readStatus( xml );
    
    Echonest::Artists artists = Echonest::Parser::parseArtists( xml );
    
    return artists;
}

Echonest::Artists Echonest::Artist::parseSimilar( QNetworkReply* reply ) throw( Echonest::ParseError )
{
    return parseSearch( reply );
}

Echonest::Artists Echonest::Artist::parseTopHottt( QNetworkReply* reply ) throw( Echonest::ParseError )
{
    return parseSearch( reply );
}

Echonest::TermList Echonest::Artist::parseTopTerms( QNetworkReply* reply ) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    
    QXmlStreamReader xml( reply->readAll() );
    
    Echonest::Parser::readStatus( xml );
    
    Echonest::TermList terms = Echonest::Parser::parseTermList( xml );
    
    return terms;
}


QUrl Echonest::Artist::setupQuery( const QByteArray& methodName, int numResults, int start ) const
{
    QUrl url = Echonest::baseGetQuery( "artist", methodName );
    if( !d->id.isEmpty() ) 
        url.addEncodedQueryItem( "id", d->id );
    else if( !d->name.isEmpty() ) {
        QString name = d->name;
        name.replace( QLatin1Char( ' ' ), QLatin1Char( '+' ) );
        url.addQueryItem( QLatin1String( "name" ), name );
    } else {
        qWarning() << "Artist method" << methodName << "called on an artist object without name or id!";
        return QUrl();
    }
    if( numResults > 0 )
        url.addEncodedQueryItem( "results", QByteArray::number( numResults ) );
    if( start >= 0 ) 
        url.addEncodedQueryItem( "start", QByteArray::number( start ) );
    
    return url;
}

QByteArray Echonest::Artist::searchParamToString(Echonest::Artist::SearchParam param)
{
    switch( param )
    {
        case Id:
            return "id";
        case Name:
            return "name";
        case Results:
            return "results";
        case Description:
            return "description";
        case FuzzyMatch:
            return "fuzzy_match";
        case MaxFamiliarity:
            return "max_familiarity";
        case MinFamiliarity:
            return "min_familiarity";
        case MaxHotttnesss:
            return "max_hotttnesss";
        case MinHotttnesss:
            return "min_hotttnesss";
        case Reverse:
            return "reverse";
        case Sort:
            return "sort";
        default:
            return "";
    }
}

void Echonest::Artist::addQueryInformation(QUrl& url, Echonest::Artist::ArtistInformation parts)
{
    if( parts.testFlag( Echonest::Artist::Audio ) )
        url.addEncodedQueryItem( "bucket", "audio" );
    if( parts.testFlag( Echonest::Artist::Biographies ) )
        url.addEncodedQueryItem( "bucket", "biographies" );
    if( parts.testFlag( Echonest::Artist::Blogs ) )
        url.addEncodedQueryItem( "bucket", "blogs" );
    if( parts.testFlag( Echonest::Artist::Familiarity ) )
        url.addEncodedQueryItem( "bucket", "familiarity" );
    if( parts.testFlag( Echonest::Artist::Hotttnesss ) )
        url.addEncodedQueryItem( "bucket", "hotttnesss" );
    if( parts.testFlag( Echonest::Artist::Images ) )
        url.addEncodedQueryItem( "bucket", "images" );
    if( parts.testFlag( Echonest::Artist::News ) )
        url.addEncodedQueryItem( "bucket", "news" );
    if( parts.testFlag( Echonest::Artist::Reviews ) )
        url.addEncodedQueryItem( "bucket", "reviews" );
    if( parts.testFlag( Echonest::Artist::Terms ) )
        url.addEncodedQueryItem( "bucket", "terms" );
    if( parts.testFlag( Echonest::Artist::Urls ) )
        url.addEncodedQueryItem( "bucket", "urls" );
    if( parts.testFlag( Echonest::Artist::Videos ) )
        url.addEncodedQueryItem( "bucket", "video" );
    if( parts.testFlag( Echonest::Artist::MusicBrainzEntries ) )
        url.addEncodedQueryItem( "bucket", "id:musicbrainz" );
    if( parts.testFlag( Echonest::Artist::SevenDigitalEntries ) )
        url.addEncodedQueryItem( "bucket", "id:7digital" );
    if( parts.testFlag( Echonest::Artist::PlaymeEntries ) )
        url.addEncodedQueryItem( "bucket", "id:playme" );
}


QDebug Echonest::operator<<(QDebug d, const Echonest::Artist& artist)
{
    return d.maybeSpace() << QString::fromLatin1( "Artist(%1, %2)" ).arg( artist.name() ).arg( QString::fromLatin1(artist.id()) );
}

