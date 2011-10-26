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

#include "CatalogSong.h"
#include "CatalogItem_p.h"

Echonest::CatalogSong::CatalogSong()
{

}

Echonest::CatalogSong::CatalogSong(const QByteArray& id, const QString& title, const QByteArray& artistId, const QString& artistName)
    : Song(id, title, artistId, artistName)
{

}

Echonest::CatalogSong::CatalogSong(const Echonest::CatalogSong& other)
    : Song(other)
    , CatalogItem( other )
{

}

Echonest::CatalogSong& Echonest::CatalogSong::operator=(const Echonest::CatalogSong& other)
{
    Song::operator=( other );
    CatalogItem::operator=( other );
    return *this;
}

Echonest::CatalogSong::~CatalogSong()
{
}

Echonest::CatalogTypes::Type Echonest::CatalogSong::type() const
{
    return Echonest::CatalogTypes::Song;
}


