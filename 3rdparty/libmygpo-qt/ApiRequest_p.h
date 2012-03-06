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
    PodcastList* toplist( uint count );
    PodcastList* search( const QString& query );
    PodcastList* suggestions( uint count );
    QNetworkReply* downloadSubscriptionsJson( const QString& username, const QString& device );
    PodcastList* podcastsOfTag( uint count, const QString& tag );
    Podcast* podcastData( const QUrl& podcasturl );
    Episode* episodeData( const QUrl& podcasturl, const QUrl& episodeurl );
    EpisodeList* favoriteEpisodes( const QString& username );
    TagList* topTags( uint count );
    AddRemoveResult* addRemoveSubscriptions( const QString& username, const QString& device, const QList< QUrl >& add, const QList< QUrl >& remove );
    Settings* accountSettings( const QString& username );
    Settings* deviceSettings( const QString& username, const QString& device );
    Settings* podcastSettings( const QString& username, const QString& podcastUrl );
    Settings* episodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl );
    Settings* setAccountSettings( const QString& username, QMap<QString, QVariant >& set, const QList<QString>& remove );
    Settings* setDeviceSettings( const QString& username, const QString& device, QMap<QString, QVariant >& set, const QList<QString>& remove );
    Settings* setPodcastSettings( const QString& username, const QString& podcastUrl, QMap<QString, QVariant >& set, const QList<QString>& remove );
    Settings* setEpisodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl, QMap<QString, QVariant >& set, const QList<QString>& remove );
    DeviceUpdates* deviceUpdates( const QString& username, const QString& deviceId, qlonglong timestamp );
    QNetworkReply* renameDevice( const QString& username, const QString& deviceId, const QString& caption, Device::Type type );
    DeviceList* listDevices( const QString& username );
    EpisodeActionList* episodeActions( const QString& username, const bool aggregated );
    EpisodeActionList* episodeActionsByPodcast( const QString& username, const QString& podcastUrl, const bool aggregated );
    EpisodeActionList* episodeActionsByDevice( const QString& username, const QString& deviceId, const bool aggregated );
    EpisodeActionList* episodeActionsByTimestamp( const QString& username, const qulonglong since );
    EpisodeActionList* episodeActionsByPodcastAndTimestamp( const QString& username, const QString& podcastUrl, const qulonglong since );
    EpisodeActionList* episodeActionsByDeviceAndTimestamp( const QString& username, const QString& deviceId, const qulonglong since );
    AddRemoveResult* uploadEpisodeActions( const QString& username, const QList<EpisodeActionPtr>& episodeActions );
    DeviceSyncResult* deviceSynchronizationStatus( const QString& username );
private:
    RequestHandler m_requestHandler;
};

}

#endif //APIREQUEST_PRIVATE_H
