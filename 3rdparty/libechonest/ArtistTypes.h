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

#ifndef ECHONEST_ARTISTTYPES_H
#define ECHONEST_ARTISTTYPES_H

#include "echonest_export.h"
#include "Util.h"

#include <QDebug>
#include <QSharedData>

class QDateTime;
class QUrl;
class AudioFileData;
class BiographyData;
class BlogData;
class ArtistImageData;
class ReviewData;
class TermData;
class VideoData;

namespace Echonest{
        
    /**
     * A link to an audio file related to an artist on the web.
     */
    class ECHONEST_EXPORT AudioFile
    {
        
    public:
        AudioFile();
        AudioFile( const AudioFile& other );
        AudioFile& operator=( const AudioFile& artist );
        ~AudioFile();
        
        /**
         * The title of the audio file.
         */
        QString title() const;
        void setTitle( const QString& title );
        
        /**
         * The artist name that this audio file is related to.
         */
        QString artist() const;
        void setArtist( const QString& );
        
        /**
         * The URL pointing to the audio file.
         */
        QUrl url() const;
        void setUrl( const QUrl& url );
        
        /**
         * The length of the referenced audio file.
         */
        qreal length() const;
        void setLength( qreal length );
        
        /**
         * The link to the website where the audio file is from.
         */
        QUrl link() const;
        void setLink( const QUrl& url );
        
        /**
         * The date that this audio was posted.
         */
        QDateTime date() const;
        void setDate( const QDateTime& );
        
        /**
         * The released album name of this audio file.
         */
        QString release() const;
        void setRelease( const QString& release );
        
        /**
         * The unique identifier for this artist audio file.
         */
        QByteArray id() const;
        void setId( const QByteArray& id );
        
    private:
        QSharedDataPointer<AudioFileData> d;
    };
    
    /**
     * A biography of an artist, including the full text content
     *  of the biography itself.
     */
    class ECHONEST_EXPORT Biography {
        
    public:
        Biography();
        Biography( const Biography& other );
        Biography& operator=( const Biography& biblio );
        ~Biography();
        
        /**
         * The URL to the biography.
         */
        QUrl url() const;
        void setUrl( const QUrl& url );
        
        /**
         * The text contents of the biography. May be very long.
         */
        QString text() const;
        void setText( const QString& text );
        
        /**
         * The site that this biography is from.
         */
        QString site() const;
        void setSite( const QString& site );
        
        /**
         * The license that this biography is licensed under.
         */
        License license() const;
        void setLicense( const License& license );
    private:
        QSharedDataPointer<BiographyData> d;
    };
    
    /**
     * A blog post about a certain artist or track.
     */
    class ECHONEST_EXPORT Blog {
        
    public:
        Blog();
        Blog( const Blog& other );
        Blog& operator=( const Blog& other );
        ~Blog();
        
        /**
         * The name of the blog or news article.
         */
        QString name() const;
        void setName( const QString& name );
        
        /**
         * The URL to the blog post or news article.
         */
        QUrl url() const;
        void setUrl( const QUrl& url );
        
        /**
         * The date when the blog post or news article was posted.
         */
        QDateTime datePosted() const;
        void setDatePosted( const QDateTime& date );
        
        /**
         * The date when this blog post or news article was found by The Echo Nest.
         */
        QDateTime dateFound() const;
        void setDateFound( const QDateTime& date );
        
        /**
         * A short summary of the blog or article.
         */
        QString summary() const;
        void setSummary( const QString& text );
        
        /**
         * The unique identifier for this entry.
         */
        QByteArray id() const;
        void setId( const QByteArray& id );
        
    private:
        QSharedDataPointer<BlogData> d;
    };
    
    /**
     * A news article about an artist.
     */
    typedef Blog NewsArticle;
    
    /**
     * An image related to an artist.
     */
    class ECHONEST_EXPORT ArtistImage {
        
    public:
        ArtistImage();
        ArtistImage( const ArtistImage& other );
        ArtistImage& operator=( const ArtistImage& img );
        ~ArtistImage();
        
        /**
         * The image url.
         */
        QUrl url() const;
        void setUrl( const QUrl& url );
        
        /**
         * The license that governs this image.
         */
        License license() const;
        void setLicense( const License& license );
        
    private:
        QSharedDataPointer<ArtistImageData> d;
    };
    
    /**
     * A review of an artist, album, or track.
     */
    class ECHONEST_EXPORT Review {
        
    public:
        Review();
        Review( const Review& other );
        Review& operator=( const Review& other );
        ~Review();
        
        /**
         * The name of the review site.
         */
        QString name() const;
        void setName( const QString& name );
        
        /**
         * The URL to the review.
         */
        QUrl url() const;
        void setUrl( const QUrl& url );
        
        /**
         * The date when the review was posted.
         */
        QDateTime dateReviewed() const;
        void setDateReviewed( const QDateTime& date );
        
        /**
         * The date when this review was found and indexed by The Echo Nest
         */
        QDateTime dateFound() const;
        void setDateFound( const QDateTime& date );
        
        /**
         * A summary of the review.
         */
        QString summary() const;
        void setSummary( const QString& text );
        
        /**
         * The url to an image associated with the review, if it exists.
         */
        QUrl imageUrl() const;
        void setImageUrl( const QUrl& imageUrl );
        
        /**
         * The album being reviewed if it is an album review, including specific release info, if it exists.
         */
        QString release() const;
        void setRelease( const QString& release );
        
        /**
         * The unique identifier for this entry.
         */
        QByteArray id() const;
        void setId( const QByteArray& id );
        
    private:
        QSharedDataPointer<ReviewData> d;
    };
    
    /**
     * A term used to describe an artist or track.
     */
    class ECHONEST_EXPORT Term {
        
    public:
        Term();
        Term( const Term& other );
        Term& operator=( const Term& img );
        ~Term();
        
        /**
         * The term name.
         */
        QString name() const;
        void setName( const QString& name );
        
        /**
         * The frequency that this term is mentioned in relation to the artist/track.
         */
        qreal frequency() const;
        void setFrequency( qreal freq );
        
        /**
         * The weight that The Echo Nest assigns to this term.
         */
        qreal weight() const;
        void setWeight( qreal weight );
        
    private:
        QSharedDataPointer<TermData> d;
    };
    
    
    /**
     * A link to a video related to an artist.
     */
    class ECHONEST_EXPORT Video {
        
    public:
        Video();
        Video( const Video& other );
        Video& operator=( const Video& other );
        ~Video();
        
        /**
         * The title of the video
         */
        QString title() const;
        void setTitle( const QString& title );
        
        /**
         * The URL to the title.
         */
        QUrl url() const;
        void setUrl( const QUrl& url );
        
        /**
         * The site that the video is from.
         */
        QString site() const;
        void setSite( const QString& site );
        
        /**
         * The date when this video was found
         */
        QDateTime dateFound() const;
        void setDateFound( const QDateTime& date );
        
        /**
         * The url to an image associated with this video, if it exists.
         */
        QUrl imageUrl() const;
        void setImageUrl( const QUrl& imageUrl );
        
        /**
         * The unique identifier for this video.
         */
        QByteArray id() const;
        void setId( const QByteArray& id );
        
    private:
        QSharedDataPointer<VideoData> d;
    };
    
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::AudioFile& artist);
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::Biography& biblio);
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::Blog& blog);
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::ArtistImage& img);
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::Review& review);
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::Term& term);
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const Echonest::Video& video);
    
    typedef QVector< AudioFile > AudioList;
    typedef QVector< Biography > BiographyList;
    typedef QVector< Blog > BlogList;
    typedef QVector< ArtistImage > ArtistImageList;
    typedef QVector< NewsArticle > NewsList;
    typedef QVector< Review > ReviewList;
    typedef QVector< Term > TermList;
    typedef QVector< Video > VideoList;
   
} // namespace
#endif
