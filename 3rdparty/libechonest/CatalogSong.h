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
#ifndef ECHONEST_CATALOG_SONG_H
#define ECHONEST_CATALOG_SONG_H

#include "echonest_export.h"
#include "Song.h"
#include "CatalogItem.h"

class CatalogSongData;
namespace Echonest {
/**
* A song that includes some additional information returned when getting a song from a catalog listing.
*/
class ECHONEST_EXPORT CatalogSong : public Song, public CatalogItem
{
public:
    CatalogSong();
    CatalogSong(const QByteArray& id, const QString& title, const QByteArray& artistId, const QString& artistName);
    CatalogSong(const Echonest::CatalogSong& other);
    virtual ~CatalogSong();
    CatalogSong& operator=( const CatalogSong& other );
    
    /**
     * The type of this item: Song.
     */
    virtual CatalogTypes::Type type() const;
    
};

typedef QVector< CatalogSong > CatalogSongs;
}


Q_DECLARE_METATYPE( Echonest::CatalogSong )
#endif
