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

#ifndef ECHONEST_ARTISTTYPES_P_H
#define ECHONEST_ARTISTTYPES_P_H

#include "Util.h"

#include <QSharedData>
#include <QString>
#include <QVector>
#include <QDate>
#include <QUrl>

class AudioFileData : public QSharedData
{
public:
    AudioFileData() {}
    AudioFileData( const AudioFileData& other ) : QSharedData( other ) {
        title = other.title;
        artist = other.artist;
        url = other.url;
        length = other.length;
        link = other.link;
        date = other.date;
        id = other.id;
        release = other.release;
    }
    
    QString title;
    QString artist;
    QUrl url;
    qreal length;
    QUrl link;
    QDateTime date;
    QString release;
    QByteArray id;
};

class BiographyData : public QSharedData
{
public:
    BiographyData() {}
    BiographyData( const BiographyData& other ) : QSharedData( other ) {
        url = other.url;
        text = other.text;
        site = other.site;
        license = other.license;
    }
    
    QUrl url;
    QString text;
    QString site;
    Echonest::License license;
};

class BlogData : public QSharedData
{
public:
    BlogData() {}
    BlogData( const BlogData& other ) : QSharedData( other ) {
        name = other.name;
        url = other.url;
        date_posted = other.date_posted;
        date_found = other.date_found;
        summary = other.summary;
        id = other.id;
    }
    
    QString name;
    QUrl url;
    QDateTime date_posted;
    QDateTime date_found;
    QString summary;
    QByteArray id;
};

class ArtistImageData : public QSharedData
{
public:
    ArtistImageData() {}
    ArtistImageData( const ArtistImageData& other ) : QSharedData( other ) {
        url = other.url;
        license = other.license;
    }
    
    QUrl url;
    Echonest::License license;
};

class ReviewData : public QSharedData
{
public:
    ReviewData() {}
    ReviewData( const ReviewData& other ) : QSharedData( other ) {
        name = other.name;
        url = other.url;
        summary = other.summary;
        date_reviewed = other.date_reviewed;
        date_found = other.date_found;
        image_url = other.image_url;
        release = other.release;
        id = other.id;
    }
    
    QString name;
    QUrl url;
    QString summary;
    QDateTime date_reviewed;
    QDateTime date_found;
    QUrl image_url;
    QString release;
    QByteArray id;
};

class TermData : public QSharedData
{
public:
    TermData() {}
    TermData( const TermData& other ) : QSharedData( other ) {   
        name = other.name;
        frequency = other.frequency;
        weight = other.weight;
    }
    
    QString name;
    qreal frequency;
    qreal weight;
};

class VideoData : public QSharedData
{
public:
    VideoData() {}
    VideoData( const VideoData& other ) : QSharedData( other ) {
        title = other.title;
        url = other.url;
        site = other.site;
        date_found = other.date_found;
        image_url = other.image_url;
        id = other.id;
    }
    
    QString title;
    QUrl url;
    QString site;
    QDateTime date_found;
    QUrl image_url;
    QByteArray id;
};


#endif
