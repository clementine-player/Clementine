/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2010 - 2011 Stefan Derkits <stefan@derkits.at>             *
* Copyright (c) 2010 - 2011 Christian Wagner <christian.wagner86@gmx.at>   *
* Copyright (c) 2010 - 2011 Felix Winter <ixos01@gmail.com>                *
*                                                                          *
* This library is free software; you can redistribute it and/or            *
* modify it under the terms of the GNU Lesser General Public               *
* License as published by the Free Software Foundation; either             *
* version 2.1 of the License, or (at your option) any later version.       *
*                                                                          *
* This library is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
* Lesser General Public License for more details.                          *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public         *
* License along with this library; if not, write to the Free Software      *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 *
* USA                                                                      *
***************************************************************************/

#include <QVariant>
#include <QList>
#include <QUrl>
#include <QString>
#include <QDateTime>

#include <qjson/serializer.h>

#include "JsonCreator.h"

using namespace mygpo;

QByteArray JsonCreator::addRemoveSubsToJSON( const QList< QUrl >& add, const QList< QUrl >& remove )
{
    QJson::Serializer serializer;
    QVariantMap jsonData;
    QVariant addVar( urlListToQVariantList( add ) );
    QVariant removeVar( urlListToQVariantList( remove ) );
    jsonData.insert( QString( QLatin1String( "add" ) ), addVar );
    jsonData.insert( QString( QLatin1String( "remove" ) ), removeVar );
    QByteArray jsonByteArray = serializer.serialize( QVariant( jsonData ) );
    return jsonByteArray;
}

QByteArray JsonCreator::saveSettingsToJSON( const QMap< QString, QVariant >& set, const QList< QString >& remove )
{
    QJson::Serializer serializer;
    QVariantMap jsonData;
    //QVariant setVar(stringMapToQVariantMap(set));
    QVariant removeVar( stringListToQVariantList( remove ) );
    jsonData.insert( QString( QLatin1String( "set" ) ), set );
    jsonData.insert( QString( QLatin1String( "remove" ) ), removeVar );
    QByteArray jsonByteArray = serializer.serialize( QVariant( jsonData ) );
    return jsonByteArray;
}

QByteArray JsonCreator::episodeActionListToJSON( const QList<EpisodeActionPtr>& episodeActions )
{
    QJson::Serializer serializer;
    QVariantList jsonData;

    foreach( const EpisodeActionPtr episodeAction, episodeActions )
    {
        jsonData.append( episodeActionToQVariantMap( episodeAction ) );
    }

    QByteArray jsonByteArray = serializer.serialize( QVariant( jsonData ) );
    return jsonByteArray;
}

QByteArray mygpo::JsonCreator::renameDeviceStringToJSON( const QString& caption, const QString& type )
{
    QJson::Serializer serializer;
    QVariantMap jsonData;
    QVariant captionVar( caption );
    QVariant typeVar( type );
    jsonData.insert( QString( QLatin1String( "caption" ) ), captionVar );
    jsonData.insert( QString( QLatin1String( "type" ) ), typeVar );
    QByteArray jsonByteArray = serializer.serialize( QVariant( jsonData ) );
    return jsonByteArray;

}

QVariantList JsonCreator::urlListToQVariantList( const QList< QUrl >& urls )
{
    QVariantList list;
    foreach( const QUrl & url, urls )
    {
        QVariant var( url.toString() );
        if( !list.contains( var ) )
            list.append( var );
    }
    return list;
}

QVariantList JsonCreator::stringListToQVariantList( const QList< QString >& strings )
{
    QVariantList list;
    foreach( const QString & str, strings )
    {
        QVariant var( str );
        list.append( var );
    }
    return list;
}

QVariantMap mygpo::JsonCreator::stringMapToQVariantMap( const QMap< QString, QString >& stringmap )
{
    QVariantMap map;
    foreach( const QString & str, stringmap.keys() )
    {
        map.insert( str, QVariant( stringmap.value( str ) ) );
    }
    return map;
}

QVariantMap JsonCreator::episodeActionToQVariantMap( const EpisodeActionPtr episodeAction )
{
    QVariantMap map;
    map.insert( QLatin1String( "podcast" ), episodeAction->podcastUrl() );
    map.insert( QLatin1String( "episode" ), episodeAction->episodeUrl() );
    if( episodeAction->deviceName().compare( QLatin1String( "" ) ) != 0 )
        map.insert( QLatin1String( "device" ), episodeAction->deviceName() );

    EpisodeAction::ActionType actionType = episodeAction->action();
    if( actionType == EpisodeAction::New )
        map.insert( QLatin1String( "action" ), QLatin1String( "new" ) );
    else if( actionType == EpisodeAction::Delete )
        map.insert( QLatin1String( "action" ), QLatin1String( "delete" ) );
    else if( actionType == EpisodeAction::Play )
        map.insert( QLatin1String( "action" ), QLatin1String( "play" ) );
    else if( actionType == EpisodeAction::Download )
        map.insert( QLatin1String( "action" ), QLatin1String( "download" ) );

    if( episodeAction->timestamp() != 0 ) {
#if QT_VERSION >= 0x040700
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(episodeAction->timestamp() );
#else
        QDateTime dateTime = QDateTime::fromTime_t(episodeAction->timestamp() / 1000 );
        QTime time = dateTime.time();
        time.addMSecs(episodeAction->timestamp() % 1000 );
        dateTime.setTime(time);
#endif
        map.insert( QLatin1String( "timestamp" ), dateTime.toString(Qt::ISODate) );
    }
    if( episodeAction->started() != 0 )
        map.insert( QLatin1String( "started" ), episodeAction->started() );
    if( episodeAction->position() != 0 )
        map.insert( QLatin1String( "position" ), episodeAction->position() );
    if( episodeAction->total() != 0 )
        map.insert( QLatin1String( "total" ), episodeAction->total() );

    return map;
}
