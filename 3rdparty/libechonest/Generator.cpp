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

#include "Generator_p.h"
#include "CatalogUpdateEntry.h"

// QJSon
#include <qjson/serializer.h>

#include <QVariant>
#include <quuid.h>
#include <QDebug>

QByteArray Echonest::Generator::catalogEntriesToJson( const Echonest::CatalogUpdateEntries& items )
{
    QJson::Serializer s;

    QVariant itms = catalogEntriesToVariant( items );
    QByteArray serialized = s.serialize( itms );
//    qDebug() << "Serialized:" << serialized;
    return serialized;
}

QByteArray Echonest::Generator::catalogEntryToJson( const Echonest::CatalogUpdateEntry& item )
{
    QJson::Serializer s;

    QVariant itm = catalogEntryToVariant( item );
    QByteArray serialized = s.serialize( itm );
   // qDebug() << "Serialized:" << serialized;
    return serialized;
}


QVariantList Echonest::Generator::catalogEntriesToVariant( const Echonest::CatalogUpdateEntries& items )
{
    // copy the catalog item into a QVariant
    QVariantList itemList;
    foreach( const Echonest::CatalogUpdateEntry& item, items )
        itemList << catalogEntryToVariant( item );

    qDebug() << "Generated " << itemList.size() << "entries to catalog variant!";
    return itemList;
}

QVariant Echonest::Generator::catalogEntryToVariant( const Echonest::CatalogUpdateEntry& item )
{
    QVariantMap itemMap;
    QVariantMap itm;

    itemMap[ QLatin1String( "action" ) ] = Echonest::catalogUpdateActionToLiteral( item.action() );

    if( item.itemId().isEmpty() )
        itm[ QLatin1String( "item_id" ) ] = QUuid::createUuid().toString().replace( QLatin1Char( '{' ), QString() ).replace( QLatin1Char( '}' ), QString() );
    else
        itm[ QLatin1String( "item_id" ) ] = item.itemId();

    if( !item.fingerprint().isEmpty() )
        itm[ QLatin1String( "fp_code" ) ] = item.fingerprint();

    if( !item.songId().isEmpty() )
        itm[ QLatin1String( "song_id" ) ] = item.songId();

    if( !item.songName().isEmpty() )
        itm[ QLatin1String( "song_name" ) ] = item.songName();

    if( !item.artistId().isEmpty() )
        itm[ QLatin1String( "artist_id" ) ] = item.artistId();

    if( !item.artistName().isEmpty() )
        itm[ QLatin1String( "artist_name" ) ] = item.artistName();

    if( !item.release().isEmpty() )
        itm[ QLatin1String( "release" ) ] = item.release();

    if( !item.genre().isEmpty() )
        itm[ QLatin1String( "genre" ) ] = item.genre();

    if( item.trackNumber() > -1 )
        itm[ QLatin1String( "track_number" ) ] = item.trackNumber();

    if( item.discNumber() > -1 )
        itm[ QLatin1String( "disc_number" ) ] = item.discNumber();

    if( !item.url().isEmpty() )
        itm[ QLatin1String( "url" ) ] = item.url();

    if( item.favoriteSet() )
        itm[ QLatin1String( "favorite" ) ] = item.favorite();

    if( item.bannedSet() )
        itm[ QLatin1String( "banned" ) ] = item.banned();

    if( item.playCount() > -1 )
        itm[ QLatin1String( "play_count" ) ] = item.playCount();

    if( item.skipCount() > -1 )
        itm[ QLatin1String( "skip_count" ) ] = item.skipCount();

    if( item.rating() > -1 )
        itm[ QLatin1String( "rating" ) ] = item.rating();

    itemMap[ QLatin1String( "item" ) ] = itm;

    return itemMap;
}
