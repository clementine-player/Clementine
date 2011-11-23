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
#ifndef CATALOG_UPDATE_ENTRY_H
#define CATALOG_UPDATE_ENTRY_H

#include "Util.h"

#include <QSharedData>
#include <QByteArray>
#include <QString>

class CatalogUpdateEntryData : public QSharedData {
    
public:
    CatalogUpdateEntryData() :  action( Echonest::CatalogTypes::Update ), track_number( -1 ), disc_number( -1 ), favorite( false ), 
                          banned( false ), play_count( -1 ), skip_count( -1 ), rating( -1 ), 
                          favoriteSet( false ), bannedSet( false ) 
    {}
    
    CatalogUpdateEntryData( const CatalogUpdateEntryData& other ) : QSharedData( other ) {
        item_id = other.item_id;
        action = other.action;
        fp_code = other.fp_code;
        
        song_id = other.song_id;
        song_name = other.song_name;
        
        artist_id = other.artist_id;
        artist_name = other.artist_name;
        
        release = other.release;
        genre = other.genre;
        track_number = other.track_number;
        disc_number = other.disc_number;
        url = other.url;
        
        favorite = other.favorite;
        banned = other.banned;
        play_count = other.play_count;
        skip_count = other.skip_count;
        rating = other.rating;
        
        favoriteSet = other.favoriteSet;
        bannedSet = other.banned;
    }
    Echonest::CatalogTypes::Action action;
    
    QByteArray item_id;
    QByteArray fp_code;
    
    QByteArray song_id;
    QString song_name;
    
    QByteArray artist_id;
    QString artist_name;
    
    QString release;
    QString genre;
    int track_number;
    int disc_number;
    QString url;
    
    bool favorite;
    bool banned;
    int play_count;
    int skip_count;
    int rating;
    
    // internal
    bool favoriteSet;
    bool bannedSet;
    
};

#endif
