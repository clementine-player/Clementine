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

#ifndef ECHONEST_CATALOG_P_H
#define ECHONEST_CATALOG_P_H

#include "CatalogArtist.h"
#include "CatalogSong.h"
#include "Util.h"

#include <QSharedData>

class CatalogData : public QSharedData
{
public:
    CatalogData() : total( 0 ), resolved( 0 ) {}
    CatalogData( const CatalogData& other ) : QSharedData( other ) {
        name = other.name;
        id = other.id;
        type = other.type;
        total = other.total;
        
        resolved = other.resolved;
        songs = other.songs;
        artists = other.artists;
    }
    ~CatalogData() {}
    
    // Fields we always get in a catalog object
    QString name;
    QByteArray id;
    Echonest::CatalogTypes::Type type;
    int total;
    
    // Fields we sometimes get depending on the query
    int resolved;
    // pending_tickets
    Echonest::CatalogSongs songs; // either has songs, or artists
    Echonest::CatalogArtists artists;
    
    
};

#endif
