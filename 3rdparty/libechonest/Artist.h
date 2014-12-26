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


#ifndef ECHONEST_ARTIST_H
#define ECHONEST_ARTIST_H


#include "ArtistTypes.h"
#include "Config.h"
#include "echonest_export.h"
#include "TypeInformation.h"
#include "Song.h"

#include <QDebug>
#include <QSharedData>
#include <QUrl>

class QNetworkReply;
class ArtistData;
class Term;
namespace Echonest{

    class Biography;
    class Catalog;

    class Artist;
    typedef QVector< Artist > Artists;

    /**
     * This encapsulates an Echo Nest artist---it always holds the basic info of artist id and
     *  artist name, and can be queried for more data.
     *
     * It is also possible to fetch more information from a given artist name or ID by creating an Artist
     *  object yourself and calling the fetch() functions directly.
     */
    class ECHONEST_EXPORT Artist
    {

    public:
        enum TermSorting {
            Weight,
            Frequency
        } ;

        /**
         *  The following are the various search parameters to the search() and similar() functions.
         *
         *  Not all are acceptable for each API call, check the API documentation at
         *   http://developer.echonest.com/docs/v4/artist.html#search for details.
         *
         *  - id                     QVector< QByteArray >             A list of the artist IDs to be searched  (e.g. [ARH6W4X1187B99274F, musicbrainz:artist:a74b1b7f-71a5-4011-9441-d0b5e4122711 ,ARH6W4X1187B99274F^2])
         *  - name                   QVector< QString >                A list of artist names to be searched (e.g. [Weezer, the beatles ,the beatles^0.5])
         *  - description            QVector< QString >                A list of descriptors [ alt-rock,-emo,harp^2 ]
         *  - results                0 < results < 200, (Default=15)   The number of results desired
         *  - min_results            0 < results < 200, (Default=0)    Indicates the minimum number of results to be returned regardless of constraints
         *  - max_familiarity        0.0 < familiarity < 1.0           The maximum familiarity for returned artists
         *  - min_familiarity        0.0 < familiarity < 1.0           The minimum familiarity for returned artists
         *  - max_hotttnesss         0.0 < hotttnesss < 1.0            The maximum hotttnesss for returned artists
         *  - min_hotttness          0.0 < hotttnesss < 1.0            The minimum hotttnesss for returned artists
         *  - reverse                [true, false]                     If true, return artists that are disimilar to the seeds
         *   -sort                   QString                           How to sort the results. Options: familiarity-asc, hotttnesss-asc, familiarity-desc, hotttnesss-desc.
         *
         */
        enum SearchParam {
            Id,
            Name,
            Results,
            Description,
            FuzzyMatch,
            MaxFamiliarity,
            MinFamiliarity,
            MaxHotttnesss,
            MinHotttnesss,
            Reverse,
            Sort,
            IdSpace,
            Mood
        };
        typedef QPair< Echonest::Artist::SearchParam, QVariant > SearchParamEntry;
        typedef QVector< SearchParamEntry > SearchParams;

        Artist();
        Artist( const QByteArray& id, const QString& name );
        explicit Artist( const QString& name );
        explicit Artist( const QByteArray& id );

        Artist( const Artist& other );
        Artist& operator=( const Artist& artist );
        virtual ~Artist();

        void init();

        QByteArray id() const;
        void setId( const QByteArray& id );

        QString name() const;
        void setName( const QString& name );

        /**
         * The following require fetching from The Echo Nest, so return a QNetworkReply*
         *  that is ready for parsing when the finished() signal is emitted.
         *
         * Call parseProfile() on the Artist object to populate it with the result of the
         *  query.
         *
         */

        /**
         * A list of audio files on the web for this artist.
         */
        AudioList audio() const;
        void setAudio( const AudioList& );

        /**
         * A list of biographies for this artist.
         */
        BiographyList biographies() const;
        void setBiographies( const BiographyList& );

        /**
         * Blogs about this artist, around the web.
         */
        BlogList blogs() const;
        void setBlogs( const BlogList& );

        /**
         * How familiar this artist is.
         */
        qreal familiarity() const;
        void setFamiliarity( qreal familiar );

        /**
         * The hotttness of this artist.
         */
        qreal hotttnesss() const;
        void setHotttnesss( qreal hotttnesss );

        /**
         * Images related to this artist.
         */
        ArtistImageList images() const;
        void setImages( const ArtistImageList& );

        /**
         * News about this artist.
         */
        NewsList news() const;
        void setNews( const NewsList& );

        /**
         * Reviews of this artist
         */
        ReviewList reviews() const;
        void setReviews( const ReviewList& );

        /**
         * Echo Nest song objects belonging to this artist.
         */
        SongList songs() const;
        void setSongs( const SongList& );

        /**
         * Terms describing this artist.
         */
        TermList terms() const;
        void setTerms( const TermList& );

        /**
         * Twitter username
         */
        QString twitter() const;
        void setTwitter( const QString& );

        /**
         * Urls pointing to this artists' basic information around the web.
         */
        QUrl lastFmUrl() const;
        void setLastFmUrl( const QUrl& );
        QUrl aolMusicUrl() const;
        void setAolMusicUrl( const QUrl& );
        QUrl amazonUrl() const;
        void setAmazonUrl( const QUrl& );
        QUrl itunesUrl() const;
        void setItunesUrl( const QUrl& );
        QUrl myspaceUrl() const;
        void setMyspaceUrl( const QUrl& );
        QUrl musicbrainzUrl() const;
        void setMusicbrainzUrl( const QUrl& url );

        /**
         * Videos related to this artist.
         */
        VideoList videos() const;
        void setVideos( const VideoList& );

        /**
         * The list of foreign ids for this artist, if fetched.
         */
        ForeignIds foreignIds() const;
        void setForeignIds( const ForeignIds& ids );

        /**
         * Fetch a list of audio documents found on the web that are related to this artist.
         *
         * @param numResults Limit how many results are returned
         * @param offset The offset of the results, if paging through results in increments.
         */
        QNetworkReply* fetchAudio( int numResults = 0, int offset = -1 ) const;

        /**
         * Fetch a list of biographies for this artist from various places on the web.
         */
        QNetworkReply* fetchBiographies( const QString& license = QString(), int numResults = 0, int offset = -1 ) const;

        /**
         * Fetch a list of blog articles relating to this artist.
         */
        QNetworkReply* fetchBlogs(  bool highRelevanceOnly = false, int numResults = 0, int offset = -1 ) const;

        /**
         * Fetch The Echo Nest's numerical estimate of how familiar this artist is to the world.
         */
        QNetworkReply* fetchFamiliarity() const;

        /**
         * Fetch the numerical description of how hot this artist is.
         *
         * Currently the only supported type is 'normal'
         */
        QNetworkReply* fetchHotttnesss( const QString& type = QLatin1String( "normal" ) ) const;

        /**
         * Fetch a list of images related to this artist.
         */
        QNetworkReply* fetchImages( const QString& license = QString(), int numResults = 0, int offset = -1 ) const;

        /**
         * Fetch a list of news articles found on the web related to this artist.
         */
        QNetworkReply* fetchNews( bool highRelevanceOnly = false, int numResults = 0, int offset = -1 ) const;

        /**
         * Fetch any number of pieces of artist information all at once.
         */
        QNetworkReply* fetchProfile( ArtistInformation information = ArtistInformation() ) const;

        /**
         * Fetch reviews related to the artist.
         */
        QNetworkReply* fetchReviews( int numResults = 0, int offset = -1 ) const;

        /**
         * Fetch a list of songs created by this artist.
         */
        QNetworkReply* fetchSongs( int numResults = 0, int offset = -1 ) const;

        /**
         * Fetch a list of the most descriptive terms for this artist.
         */
        QNetworkReply* fetchTerms( TermSorting sorting = Frequency ) const;

        /**
         * Fetch artist's twitter handle.
         */
        QNetworkReply* fetchTwitter() const;

        /**
         * Fetch links to the artist's official site, MusicBrainz site, MySpace site, Wikipedia article, Amazon list, and iTunes page.
         */
        QNetworkReply* fetchUrls() const;

        /**
         * Fetch a list of video documents found on the web related to an artist.
         */
        QNetworkReply* fetchVideo( int numResults = 0, int offset = -1 ) const;


        /**
         * Parse a completed QNetworkReply* that has fetched more information about this artist.
         *  This will update the artist object with the new values that have been fetched.
         *
         * @return The number of results available on the server.
         */
        int parseProfile( QNetworkReply* ) throw( ParseError );

        /**
         * Fetch a list of similar artists given one or more artists for comparison.
         *
         * Up to five artist names or ids can be included for the similarity search.
         *
         *
         * So they are passed as a list of [paramname, paramvalue] to be included in the query.
         *
         * Boosting: This method can take multiple seed artists. You an give a seed artist more or less weight by boosting the artist. A boost is an
         *  affinity for a seed that gives it more or less weight when making calculations based on the argument. In case seeds are not meant to be equally
         *  valued, the boost can help clarify where along a spectrum each argument falls. The boost is a positive floating point value, where 1 gives the normal
         *  weight. It is signified by appending a caret and weight to the argument.
         *
         * See http://developer.echonest.com/docs/v4/artist.html#similar for boosting examples.
         *
         * Call parseSimilar() once the returned QNetworkReply* has emitted its finished() signal
         */
        static QNetworkReply* fetchSimilar( const SearchParams& params, ArtistInformation information = ArtistInformation(),  int numResults = 0, int offset = -1 );

        /**
         * Search for artists.
         *
         * Warning: If limit is set to true, at least one idspace must also be provided.
         *
         * One of name or description is required, but only one can be used in a query at one time
         *
         */
        static QNetworkReply* search( const SearchParams& params, ArtistInformation information = ArtistInformation(), bool limit = false );

        /**
         * Fetch a list of the current top artists in terms of hotttnesss.
         *
         * Warning If limit is set to true, at least one idspace must also be provided in the bucket parameter.
         *
         */
        static QNetworkReply* topHottt( ArtistInformation information = ArtistInformation(), int numResults = 0, int offset = -1, bool limit = false );

        /**
         * Fetch a list of the top overall terms.
         */
        static QNetworkReply* topTerms( int numResults = 15 );

        /**
         * Suggest artists based on a partial name.
         *
         * \param results How many results to return, between 0 and 15. Default is 10
         */
        static QNetworkReply* suggest( const QString& name, int results = 10 );

        /**
         * Returns a list of terms of the given type, for use in other calls.
         *
         * \param type Which type of term to return, at the moment only 'style' or 'mood'
         */
        static QNetworkReply* listTerms( const QString& type = QLatin1String("style") );

        /**
         * Parse the result of a fetchSimilar() call, which returns a list of artists similar to the
         *  original pair.
         */
        static Artists parseSimilar( QNetworkReply* ) throw( ParseError );

        /**
         * Parse the result of an artist search.
         */
        static Artists parseSearch( QNetworkReply* ) throw( ParseError );

        /**
         * Parse the result of a top hottness query.
         */
        static Artists parseTopHottt( QNetworkReply* ) throw( ParseError );

        /**
         * Parse the result of a top terms query.
         */
        static TermList parseTopTerms( QNetworkReply* ) throw( ParseError );

        /**
         * Parse the result of a suggest query
         *
         * Returns a list of suggested artists
         */
        static Artists parseSuggest( QNetworkReply* ) throw( ParseError );

        /**
         * Parse the result of a termList query. Returns the list of values returned.
         */
        static QVector< QString > parseTermList( QNetworkReply* ) throw( ParseError );

    private:
        QUrl setupQuery( const QByteArray& methodName, int numResults = 0, int start = -1 ) const;

        static QByteArray searchParamToString( SearchParam param );
        static void addQueryInformation( QUrl& url, ArtistInformation information );

        QSharedDataPointer<ArtistData> d;

        friend class Catalog;
    };

    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::Artist& artist);

} // namespace

Q_DECLARE_METATYPE( Echonest::Artist )

#endif
