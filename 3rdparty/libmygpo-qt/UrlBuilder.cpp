/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2010 - 2013 Stefan Derkits <stefan@derkits.at>             *
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


#include "UrlBuilder.h"
#include "Config.h"

#include <QString>
#include <QStringBuilder>
#include <QLatin1String>

using namespace mygpo;

const QString UrlBuilder::s_server = Config::instance()->mygpoBaseUrl().toString();
const QString UrlBuilder::s_api2 = QLatin1String( "/api/2" );
const QString UrlBuilder::s_api1 = QLatin1String( "/api/1" );

static QString getFormatExtension( UrlBuilder::Format f )
{
    QString ret;
    switch( f )
    {
    case UrlBuilder::JSON:
        ret =  QString( QLatin1String( ".json" ) );
        break;
    case UrlBuilder::OPML:
        ret =  QString( QLatin1String( ".opml" ) );
        break;
    case UrlBuilder::TEXT:
        ret =  QString( QLatin1String( ".txt" ) );
        break;
    case UrlBuilder::XML:
        ret = QString( QLatin1String( ".xml" ) );
        break;
    }
    return ret;
}

QString UrlBuilder::getToplistUrl( uint i, Format f )
{
    QString numString = QString::number(( i == 0 ) ? 1 : i );
    return s_server % QLatin1String( "/toplist/" ) % numString % getFormatExtension( f );
}

QString UrlBuilder::getSuggestionsUrl( uint i, Format f )
{
    QString numString = QString::number(( i == 0 ) ? 1 : i );
    return s_server % QLatin1String( "/suggestions/" ) % numString % getFormatExtension( f );
}

QString UrlBuilder::getPodcastSearchUrl( const QString& query, Format f )
{
    return s_server % QLatin1String( "/search" ) % getFormatExtension( f ) % QLatin1String( "?q=" ) % query;
}

QString UrlBuilder::getSubscriptionsUrl( const QString& username, const QString& device, UrlBuilder::Format f)
{
    QString deviceString = device.isNull() ? QString(QLatin1String("")) : (QLatin1String( "/" ) % device);
    return s_server % QLatin1String( "/subscriptions/" ) % username % deviceString % getFormatExtension( f );
}

QString UrlBuilder::getTopTagsUrl( uint i )
{
    QString numString = QString::number(( i == 0 ) ? 1 : i );
    return s_server % s_api2 % QLatin1String( "/tags/" ) % numString % QLatin1String( ".json" );
}

QString UrlBuilder::getPodcastsOfTagUrl( const QString& tag, uint i )
{
    QString numString = QString::number(( i == 0 ) ? 1 : i );
    return s_server % s_api2 % QLatin1String( "/tag/" ) % tag % QLatin1String( "/" ) % numString % QLatin1String( ".json" );
}

QString UrlBuilder::getPodcastDataUrl( const QString& url )
{
    return s_server % s_api2 % QLatin1String( "/data/podcast" ) % QLatin1String( ".json" ) % QLatin1String( "?url=" ) % url;
}

QString UrlBuilder::getEpisodeDataUrl( const QString& podcastUrl, const QString& episodeUrl )
{
    return s_server % s_api2 % QLatin1String( "/data/episode" ) % QLatin1String( ".json" ) % QLatin1String( "?podcast=" ) % podcastUrl % QLatin1String( "&url=" ) % episodeUrl;
}

QString UrlBuilder::getFavEpisodesUrl( const QString& username )
{
    return s_server % s_api2 % QLatin1String( "/favorites/" ) % username % QLatin1String( ".json" );
}


QString UrlBuilder::getAddRemoveSubUrl( const QString& username, const QString& deviceId )
{
    return s_server % s_api2 % QLatin1String( "/subscriptions/" ) % username % QLatin1String( "/" ) % deviceId % QLatin1String( ".json" );
}

QString UrlBuilder::getAccountSettingsUrl( const QString& username )
{
    return s_server % s_api2 % QLatin1String( "/settings/" ) % username % QLatin1String( "/account" ) % QLatin1String( ".json" );
}

QString UrlBuilder::getDeviceSettingsUrl( const QString& username, const QString& deviceId )
{
    return s_server % s_api2 % QLatin1String( "/settings/" ) % username % QLatin1String( "/device" ) % QLatin1String( ".json" ) % QLatin1String( "?device=" ) % deviceId;
}

QString UrlBuilder::getPodcastSettingsUrl( const QString& username, const QString& podcastUrl )
{
    return s_server % s_api2 % QLatin1String( "/settings/" ) % username % QLatin1String( "/podcast" ) % QLatin1String( ".json" ) % QLatin1String( "?podcast=" ) % podcastUrl;
}

QString UrlBuilder::getEpisodeSettingsUrl( const QString& username, const QString& podcastUrl, const QString& episodeUrl )
{
    return s_server % s_api2 % QLatin1String( "/settings/" ) % username % QLatin1String( "/episode" ) % QLatin1String( ".json" ) % QLatin1String( "?podcast=" ) % podcastUrl % QLatin1String( "&episode=" ) % episodeUrl;
}

QString UrlBuilder::getDeviceListUrl( const QString& username )
{
    return s_server % s_api2 % QLatin1String( "/devices/" ) % username % QLatin1String( ".json" ) ;
}

QString UrlBuilder::getDeviceUpdatesUrl( const QString& username, const QString& deviceId, qulonglong timestamp )
{
    QString numString = QString::number( timestamp );
    return s_server % s_api2 % QLatin1String( "/updates/" ) % username % QLatin1String( "/" ) % deviceId % QLatin1String( ".json?since=" ) % numString;
}

QString UrlBuilder::getRenameDeviceUrl( const QString& username, const QString& deviceId )
{
    return s_server % s_api2 % QLatin1String( "/devices/" ) % username % QLatin1String( "/" ) % deviceId % QLatin1String( ".json" );
}

QString UrlBuilder::getEpisodeActionsUrl( const QString& username, const bool aggregated )
{
    QString agg;
    if( aggregated )
        agg = QLatin1String( "?aggregated=true" );
    else
        agg = QLatin1String( "" );

    return s_server % s_api2 % QLatin1String( "/episodes/" ) % username % QLatin1String( ".json" ) % agg;
}

QString UrlBuilder::getEpisodeActionsUrlByPodcast( const QString& username, const QString& podcastUrl, const bool aggregated )
{
    QString agg;
    if( aggregated )
        agg = QLatin1String( "&aggregated=true" );
    else
        agg = QLatin1String( "" );

    return s_server % s_api2 % QLatin1String( "/episodes/" ) % username % QLatin1String( ".json?podcast=" ) % podcastUrl % agg;
}

QString UrlBuilder::getEpisodeActionsUrlByDevice( const QString& username, const QString& deviceId, bool aggregated )
{
    QString agg;
    if( aggregated )
        agg = QLatin1String( "&aggregated=true" );
    else
        agg = QLatin1String( "" );

    return s_server % s_api2 % QLatin1String( "/episodes/" ) % username % QLatin1String( ".json?device=" ) % deviceId % agg;
}

QString UrlBuilder::getEpisodeActionsUrlByTimestamp( const QString& username, qulonglong since )
{
    QString numString = QString::number( since );
    return s_server % s_api2 % QLatin1String( "/episodes/" ) % username % QLatin1String( ".json?since=" ) % numString;
}

QString UrlBuilder::getEpisodeActionsUrlByPodcastAndTimestamp( const QString& username, const QString& podcastUrl, qulonglong since )
{
    QString numString = QString::number( since );
    return s_server % s_api2 % QLatin1String( "/episodes/" ) % username % QLatin1String( ".json?podcast=" ) % podcastUrl % QLatin1String( "&since=" ) % numString;
}

QString UrlBuilder::getEpisodeActionsUrlByDeviceAndTimestamp( const QString& username, const QString& deviceId, qulonglong since )
{
    QString numString = QString::number( since );
    return s_server % s_api2 % QLatin1String( "/episodes/" ) % username % QLatin1String( ".json?device=" ) % deviceId % QLatin1String( "&since=" ) % numString;
}

QString UrlBuilder::getUploadEpisodeActionsUrl( const QString& username )
{
    return s_server % s_api2 % QLatin1String( "/episodes/" ) % username % QLatin1String( ".json" );
}

QString UrlBuilder::getDeviceSynchronizationStatusUrl ( const QString& username )
{
    return s_server % s_api2 % QLatin1String( "/sync-devices/" ) % username % QLatin1String( ".json" );
}
