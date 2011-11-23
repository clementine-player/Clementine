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
#ifndef ECHONEST_CATALOG_ARTIST_H
#define ECHONEST_CATALOG_ARTIST_H

#include "echonest_export.h"
#include "Artist.h"
#include "CatalogItem.h"

class CatalogArtistData;
namespace Echonest {
    
/**
 * An artist that includes some additional information returned when getting an artist from a catalog listing.
 */
class ECHONEST_EXPORT CatalogArtist : public Artist, public CatalogItem
{
public:
    CatalogArtist();
    explicit CatalogArtist(const QString& name);
    CatalogArtist(const QByteArray& id, const QString& name);
    CatalogArtist(const Echonest::CatalogArtist& other);
    virtual ~CatalogArtist();
    CatalogArtist& operator=( const CatalogArtist& other );
    
    /**
     * The type of this catalog item: Artist.
     */
    virtual CatalogTypes::Type type() const;
};

typedef QVector< CatalogArtist > CatalogArtists;

}

Q_DECLARE_METATYPE( Echonest::CatalogArtist )
#endif
