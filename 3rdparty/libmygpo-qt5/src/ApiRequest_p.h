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

#ifndef APIREQUEST_PRIVATE_H
#define APIREQUEST_PRIVATE_H

#include "ApiRequest.h"

#include "RequestHandler.h"

namespace mygpo
{

class ApiRequestPrivate
{
public:
    //Constructors
    ApiRequestPrivate( const QString& username, const QString& password, QNetworkAccessManager* nam );
    ApiRequestPrivate( QNetworkAccessManager* nam );
    //Member Functions
    QNetworkReply* toplistOpml( uint count );
    QNetworkReply* searchOpml( const QString& query );
    QNetworkReply* suggestionsOpml( uint count );
    QNetworkReply* downloadSubscriptionsOpml( const QString& username, const QString& device );
    QNetworkReply* toplistTxt( uint count );
    QNetworkReply* searchTxt( const QString& query );
    QNetworkReply* suggestionsTxt( uint count );
    QNetworkReply* downloadSubscriptionsTxt ( const QString& username, const QString& device );
    QNetworkReply* toplistXml( uint count );
    QNetworkReply* searchXml( const QString& query );
    PodcastListPtr toplist( uint count );
    PodcastListPtr search( const QString& query );
    PodcastListPtr suggestions( uint count );
    QNetworkReply* downloadSubscriptionsJson( const QString& username, const QString& device );
    PodcastListPtr podcastsOfTag( uint count, const QString& tag );
    PodcastPtr podcastData( const QUrl& podcasturl );
    EpisodePtr episodeData( const QUrl& podcasturl, const QUrl& episodeurl );
    EpisodeListPtr favoriteEpisodes( const QString& username );
    TagListPtr topTags( uint count );
    AddRemoveResultPtr addRemoveSubscriptions( const QString& username, const QString& device, const QList< QUrl >& add, const QList< QUrl >& remove );
    SettingsPtr accountSettings( const QString& username );
    SettingsPtr deviceSettings( const QString& username, const QString& device );
    SettingsPtr podcastSettings( const QString& username, const QString& podcastUrl );
    SettingsPtr episodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl );
    SettingsPtr setAccountSettings( const QString& username, QMap<QString, QVariant >& set, const QList<QString>& remove );
    SettingsPtr setDeviceSettings( const QString& username, const QString& device, QMap<QString, QVariant >& set, const QList<QString>& remove );
    SettingsPtr setPodcastSettings( const QString& username, const QString& podcastUrl, QMap<QString, QVariant >& set, const QList<QString>& remove );
    SettingsPtr setEpisodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl, QMap<QString, QVariant >& set, const QList<QString>& remove );
    DeviceUpdatesPtr deviceUpdates( const QString& username, const QString& deviceId, qlonglong timestamp );
    QNetworkReply* renameDevice( const QString& username, const QString& deviceId, const QString& caption, Device::Type type );
    DeviceListPtr listDevices( const QString& username );
    EpisodeActionListPtr episodeActions( const QString& username, const bool aggregated );
    EpisodeActionListPtr episodeActionsByPodcast( const QString& username, const QString& podcastUrl, const bool aggregated );
    EpisodeActionListPtr episodeActionsByDevice( const QString& username, const QString& deviceId, const bool aggregated );
    EpisodeActionListPtr episodeActionsByTimestamp( const QString& username, const qulonglong since );
    EpisodeActionListPtr episodeActionsByPodcastAndTimestamp( const QString& username, const QString& podcastUrl, const qulonglong since );
    EpisodeActionListPtr episodeActionsByDeviceAndTimestamp( const QString& username, const QString& deviceId, const qulonglong since );
    AddRemoveResultPtr uploadEpisodeActions( const QString& username, const QList<EpisodeActionPtr>& episodeActions );
    DeviceSyncResultPtr deviceSynchronizationStatus( const QString& username );
    DeviceSyncResultPtr setDeviceSynchronizationStatus( const QString& username, const QList<QStringList>& synchronize, const QList<QString>& stopSynchronize );
private:
    RequestHandler m_requestHandler;
};

}

#endif //APIREQUEST_PRIVATE_H