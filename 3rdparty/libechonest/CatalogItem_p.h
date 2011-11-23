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

#ifndef ECHONEST_CATALOG_ITEM_P_H
#define ECHONEST_CATALOG_ITEM_P_H

#include "Song_p.h"

#include <QSharedData>
#include <QString>
#include <QVector>
#include <QDateTime>
#include "CatalogUpdateEntry.h"


class CatalogItemData : public QSharedData
{
public:
    CatalogItemData() {}
    
    CatalogItemData(const CatalogItemData& other) : QSharedData( other )
    {
        foreign_id = other.foreign_id;
        request = other.request;
        date_added = other.date_added;
        rating = other.rating;
        play_count = other.play_count;
    }
    ~CatalogItemData() {}
    
    Echonest::CatalogUpdateEntry request;
    QByteArray foreign_id;
    QDateTime date_added;
    int rating;
    int play_count;
};

#endif
