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

#include "CatalogArtist.h"
#include "CatalogItem_p.h"

Echonest::CatalogArtist::CatalogArtist()
{

}

Echonest::CatalogArtist::CatalogArtist(const QString& name)
    : Artist( name )
{

}

Echonest::CatalogArtist::CatalogArtist(const QByteArray& id, const QString& name)
    : Artist(id, name)
{

}

Echonest::CatalogArtist::CatalogArtist(const Echonest::CatalogArtist& other)
    : Artist( other )
    , CatalogItem( other )
{

}

Echonest::CatalogArtist::~CatalogArtist()
{

}

Echonest::CatalogArtist& Echonest::CatalogArtist::operator=(const Echonest::CatalogArtist& other)
{
    Artist::operator=( other );
    CatalogItem::operator=( other );
    return *this;
}

Echonest::CatalogTypes::Type Echonest::CatalogArtist::type() const
{
    return Echonest::CatalogTypes::Artist;
}
