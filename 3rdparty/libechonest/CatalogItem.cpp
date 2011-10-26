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

#include "CatalogItem.h"
#include "CatalogItem_p.h"

// just an interface.

Echonest::CatalogItem::CatalogItem()
    : dd( new CatalogItemData )
{

}

Echonest::CatalogItem::CatalogItem(const Echonest::CatalogItem& other)
    : dd( other.dd )
{
}

Echonest::CatalogItem::~CatalogItem()
{

}

Echonest::CatalogItem& Echonest::CatalogItem::operator=(const Echonest::CatalogItem& other)
{
    dd = other.dd;
    return *this;
}


QDateTime Echonest::CatalogItem::dateAdded() const
{
    return dd->date_added;
}

void Echonest::CatalogItem::setDateAdded(const QDateTime& dt)
{
    dd->date_added = dt;
}

QByteArray Echonest::CatalogItem::foreignId() const
{
    return dd->foreign_id;
}

void Echonest::CatalogItem::setForeignId(const QByteArray& id)
{
    dd->foreign_id = id;
}

Echonest::CatalogUpdateEntry Echonest::CatalogItem::request() const
{
    return dd->request;
}

void Echonest::CatalogItem::setRequest(const Echonest::CatalogUpdateEntry& request)
{
    dd->request = request;
}

int Echonest::CatalogItem::rating() const
{
    return dd->rating;
}

void Echonest::CatalogItem::setRating(int rating)
{
    dd->rating = rating;
}

int Echonest::CatalogItem::playCount() const
{
    return dd->play_count;
}

void Echonest::CatalogItem::setPlayCount(int count)
{
    dd->play_count = count;
}
