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

#ifndef ECHONEST_ARTIST_P_H
#define ECHONEST_ARTIST_P_H

#include "Song.h"
#include "ArtistTypes.h"

#include <QSharedData>
#include <QString>
#include <QVector>

namespace Echonest {
    class Artist;
}

class ArtistData : public QSharedData
{
public:
    ArtistData() : familiarity( -1 ), hotttnesss( -1 ) {}
    ArtistData( const QByteArray& id, const QString& name ) : id( id ), name( name ), familiarity( -1 ), hotttnesss( -1 ) {}
    ArtistData(const ArtistData& other) : QSharedData( other )
    {
        id = other.id;
        name = other.name;
        
        audio = other.audio;
        biographies = other.biographies;
        blogs = other.blogs;
        
        familiarity = other.familiarity;
        hotttnesss = other.hotttnesss;
        
        images = other.images;
        news = other.news;
        reviews = other.reviews;
        songs = other.songs;
        similar = other.similar;
        terms = other.terms;
        videos = other.videos;
        
        twitter = other.twitter;
        
        lastfm_url = other.lastfm_url;
        aolmusic_url = other.aolmusic_url;
        myspace_url = other.myspace_url;
        amazon_url = other.amazon_url;
        itunes_url = other.itunes_url;
        mb_url = other.mb_url;
        
        foreign_ids = other.foreign_ids;
    }
    
    // The following exist in all valid Artist objects
    QByteArray id;
    QString name;
    
    //The following are populated on demand, and may not exist
    Echonest::AudioList audio;
    Echonest::BiographyList biographies;
    Echonest::BlogList blogs;
    
    qreal familiarity;
    qreal hotttnesss;
    
    Echonest::ArtistImageList images;
    Echonest::NewsList news;
    Echonest::ReviewList reviews;
    Echonest::SongList songs;
    QVector<Echonest::Artist> similar;
    Echonest::TermList terms;
    Echonest::VideoList videos;
    QString twitter;
    
    QUrl lastfm_url;
    QUrl aolmusic_url;
    QUrl myspace_url;
    QUrl amazon_url;
    QUrl itunes_url;
    QUrl mb_url;
    
    Echonest::ForeignIds foreign_ids;
};

#endif

