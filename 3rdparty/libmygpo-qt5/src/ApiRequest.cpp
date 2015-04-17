/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2010 - 2014 Stefan Derkits <stefan@derkits.at>             *
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


#include "ApiRequest.h"

#include "ApiRequest_p.h"

#include "UrlBuilder.h"
#include "JsonCreator.h"

#include <QString>
#include <QByteArray>
#include <QLatin1String>

using namespace mygpo;

ApiRequestPrivate::ApiRequestPrivate( const QString& username, const QString& password, QNetworkAccessManager* nam ) : m_requestHandler( username, password, nam )
{

}

ApiRequestPrivate::ApiRequestPrivate( QNetworkAccessManager* nam ) : m_requestHandler( nam )
{

}

QNetworkReply* ApiRequestPrivate::toplistOpml( uint count )
{
    QString requestUrl = UrlBuilder::getToplistUrl( count, UrlBuilder::OPML );
    return m_requestHandler.getRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::searchOpml( const QString& query )
{
    QString requestUrl = UrlBuilder::getPodcastSearchUrl( query, UrlBuilder::OPML );
    return m_requestHandler.getRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::suggestionsOpml( uint count )
{
    QString requestUrl = UrlBuilder::getSuggestionsUrl( count , UrlBuilder::OPML );
    return m_requestHandler.authGetRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::downloadSubscriptionsOpml( const QString& username, const QString& device )
{
    QString requestUrl = UrlBuilder::getSubscriptionsUrl( username, device, UrlBuilder::OPML );
    return m_requestHandler.authGetRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::toplistTxt( uint count )
{
    QString requestUrl = UrlBuilder::getToplistUrl( count, UrlBuilder::TEXT );
    return m_requestHandler.getRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::searchTxt( const QString& query )
{
    QString requestUrl = UrlBuilder::getPodcastSearchUrl( query, UrlBuilder::TEXT );
    return m_requestHandler.getRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::suggestionsTxt( uint count )
{
    QString requestUrl = UrlBuilder::getSuggestionsUrl( count , UrlBuilder::TEXT );
    return m_requestHandler.authGetRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::downloadSubscriptionsTxt(const QString& username, const QString& device)
{
    QString requestUrl = UrlBuilder::getSubscriptionsUrl( username, device, UrlBuilder::TEXT );
    return m_requestHandler.authGetRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::toplistXml ( uint count )
{
    QString requestUrl = UrlBuilder::getToplistUrl( count, UrlBuilder::XML );
    return m_requestHandler.getRequest( requestUrl );
}

QNetworkReply* ApiRequestPrivate::searchXml( const QString& query )
{
    QString requestUrl = UrlBuilder::getPodcastSearchUrl( query, UrlBuilder::XML );
    return m_requestHandler.getRequest( requestUrl );
}

PodcastListPtr ApiRequestPrivate::toplist( uint count )
{
    QString requestUrl = UrlBuilder::getToplistUrl( count );
    QNetworkReply *reply;
    reply = m_requestHandler.getRequest( requestUrl );
    PodcastListPtr podcastList( new PodcastList( reply ) );
    return podcastList;
}

PodcastListPtr ApiRequestPrivate::search( const QString& query )
{
    QString requestUrl = UrlBuilder::getPodcastSearchUrl( query );
    QNetworkReply *reply;
    reply = m_requestHandler.getRequest( requestUrl );

    PodcastListPtr podcastList( new PodcastList( reply ) );
    return podcastList;
}

PodcastListPtr ApiRequestPrivate::suggestions( uint count )
{
    QString requestUrl = UrlBuilder::getSuggestionsUrl( count );
    QNetworkReply *reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    PodcastListPtr podcastList( new PodcastList( reply ) );
    return podcastList;
}

QNetworkReply* ApiRequestPrivate::downloadSubscriptionsJson(const QString& username, const QString& device)
{
    QString requestUrl = UrlBuilder::getSubscriptionsUrl( username, device );
    return m_requestHandler.authGetRequest( requestUrl );
}

EpisodePtr ApiRequestPrivate::episodeData( const QUrl& podcasturl, const QUrl& episodeurl )
{
    QString requestUrl = UrlBuilder::getEpisodeDataUrl( podcasturl.toString(), episodeurl.toString() );
    QNetworkReply *reply;
    reply = m_requestHandler.getRequest( requestUrl );

    EpisodePtr episode( new Episode( reply ) );
    return episode;
}

EpisodeListPtr ApiRequestPrivate::favoriteEpisodes( const QString& username )
{
    QString requestUrl = UrlBuilder::getFavEpisodesUrl( username );
    QNetworkReply *reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    EpisodeListPtr episodeList( new EpisodeList( reply ) );
    return episodeList;
}

PodcastPtr ApiRequestPrivate::podcastData( const QUrl& podcasturl )
{
    QString requestUrl = UrlBuilder::getPodcastDataUrl( podcasturl.toString() );
    QNetworkReply *reply;
    reply = m_requestHandler.getRequest( requestUrl );

    PodcastPtr podcast( new Podcast( reply ) );
    return podcast;
}

PodcastListPtr ApiRequestPrivate::podcastsOfTag( uint count, const QString& tag )
{
    QString requestUrl = UrlBuilder::getPodcastsOfTagUrl( tag, count );
    QNetworkReply *reply;
    reply = m_requestHandler.getRequest( requestUrl );

    PodcastListPtr podcastList( new PodcastList( reply ) );
    return podcastList;
}

TagListPtr ApiRequestPrivate::topTags( uint count )
{
    QString requestUrl = UrlBuilder::getTopTagsUrl( count );
    QNetworkReply *reply;
    reply = m_requestHandler.getRequest( requestUrl );

    TagListPtr tagList( new TagList( reply ) );
    return tagList;
}

AddRemoveResultPtr ApiRequestPrivate::addRemoveSubscriptions( const QString& username, const QString& device, const QList< QUrl >& add, const QList< QUrl >& remove )
{
    QString requestUrl = UrlBuilder::getAddRemoveSubUrl( username, device );
    QByteArray data = JsonCreator::addRemoveSubsToJSON( add, remove );
    QNetworkReply *reply;
    reply = m_requestHandler.postRequest( data, requestUrl );
    AddRemoveResultPtr addRemoveResult( new AddRemoveResult( reply ) );
    return addRemoveResult;
}

SettingsPtr ApiRequestPrivate::accountSettings( const QString& username )
{
    QString requestUrl = UrlBuilder::getAccountSettingsUrl( username );
    QNetworkReply *reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

SettingsPtr ApiRequestPrivate::deviceSettings( const QString& username, const QString& device )
{
    QString requestUrl = UrlBuilder::getDeviceSettingsUrl( username, device );
    QNetworkReply *reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

SettingsPtr ApiRequestPrivate::podcastSettings( const QString& username, const QString& podcastUrl )
{
    QString requestUrl = UrlBuilder::getPodcastSettingsUrl( username, podcastUrl );
    QNetworkReply *reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

SettingsPtr ApiRequestPrivate::episodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl )
{
    QString requestUrl = UrlBuilder::getEpisodeSettingsUrl( username, podcastUrl, episodeUrl );
    QNetworkReply *reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

SettingsPtr ApiRequestPrivate::setAccountSettings( const QString& username, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    QString requestUrl = UrlBuilder::getAccountSettingsUrl( username );
    QNetworkReply *reply;
    QByteArray postData = JsonCreator::saveSettingsToJSON( set, remove );
    reply = m_requestHandler.postRequest( postData, requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

SettingsPtr ApiRequestPrivate::setDeviceSettings( const QString& username, const QString& device, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    QString requestUrl = UrlBuilder::getDeviceSettingsUrl( username, device );
    QNetworkReply *reply;
    QByteArray postData = JsonCreator::saveSettingsToJSON( set, remove );
    reply = m_requestHandler.postRequest( postData, requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

SettingsPtr ApiRequestPrivate::setPodcastSettings( const QString& username, const QString& podcastUrl, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    QString requestUrl = UrlBuilder::getPodcastSettingsUrl( username, podcastUrl );
    QNetworkReply *reply;
    QByteArray postData = JsonCreator::saveSettingsToJSON( set, remove );
    reply = m_requestHandler.postRequest( postData, requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

SettingsPtr ApiRequestPrivate::setEpisodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    QString requestUrl = UrlBuilder::getEpisodeSettingsUrl( username, podcastUrl, episodeUrl );
    QNetworkReply *reply;
    QByteArray postData = JsonCreator::saveSettingsToJSON( set, remove );
    reply = m_requestHandler.postRequest( postData, requestUrl );
    SettingsPtr settings( new Settings( reply ) );
    return settings;
}

DeviceUpdatesPtr ApiRequestPrivate::deviceUpdates( const QString& username, const QString& deviceId, qlonglong timestamp )
{
    QString requestUrl = UrlBuilder::getDeviceUpdatesUrl( username, deviceId, timestamp );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    DeviceUpdatesPtr updates( new DeviceUpdates( reply ) );
    return updates;
}

EpisodeActionListPtr ApiRequestPrivate::episodeActions( const QString& username, const bool aggregated )
{
    QString requestUrl = UrlBuilder::getEpisodeActionsUrl( username, aggregated );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    EpisodeActionListPtr episodeActions( new EpisodeActionList( reply ) );
    return episodeActions;
}

EpisodeActionListPtr ApiRequestPrivate::episodeActionsByPodcast( const QString& username, const QString& podcastUrl, const bool aggregated )
{
    QString requestUrl = UrlBuilder::getEpisodeActionsUrlByPodcast( username, podcastUrl, aggregated );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    EpisodeActionListPtr episodeActions( new EpisodeActionList( reply ) );
    return episodeActions;
}

EpisodeActionListPtr ApiRequestPrivate::episodeActionsByDevice( const QString& username, const QString& deviceId, const bool aggregated )
{
    QString requestUrl = UrlBuilder::getEpisodeActionsUrlByDevice( username, deviceId, aggregated );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    EpisodeActionListPtr episodeActions( new EpisodeActionList( reply ) );
    return episodeActions;
}

EpisodeActionListPtr ApiRequestPrivate::episodeActionsByTimestamp( const QString& username, const qulonglong since )
{
    QString requestUrl = UrlBuilder::getEpisodeActionsUrlByTimestamp( username, since );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    EpisodeActionListPtr episodeActions( new EpisodeActionList( reply ) );
    return episodeActions;
}

EpisodeActionListPtr ApiRequestPrivate::episodeActionsByPodcastAndTimestamp( const QString& username, const QString& podcastUrl, const qulonglong since )
{
    QString requestUrl = UrlBuilder::getEpisodeActionsUrlByPodcastAndTimestamp( username, podcastUrl, since );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    EpisodeActionListPtr episodeActions( new EpisodeActionList( reply ) );
    return episodeActions;
}

EpisodeActionListPtr ApiRequestPrivate::episodeActionsByDeviceAndTimestamp( const QString& username, const QString& deviceId, const qulonglong since )
{
    QString requestUrl = UrlBuilder::getEpisodeActionsUrlByDeviceAndTimestamp( username, deviceId, since );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    EpisodeActionListPtr episodeActions( new EpisodeActionList( reply ) );
    return episodeActions;
}

AddRemoveResultPtr ApiRequestPrivate::uploadEpisodeActions( const QString& username, const QList<EpisodeActionPtr>& episodeActions )
{
    QString requestUrl = UrlBuilder::getEpisodeActionsUrl( username, false );
    QNetworkReply *reply;
    QByteArray postData = JsonCreator::episodeActionListToJSON( episodeActions );
    reply = m_requestHandler.postRequest( postData, requestUrl );
    AddRemoveResultPtr addRemoveResult( new AddRemoveResult( reply ) );
    return addRemoveResult;
}

QNetworkReply* ApiRequestPrivate::renameDevice( const QString& username , const QString& deviceId , const QString& caption, Device::Type type )
{
    QString requestUrl = UrlBuilder::getRenameDeviceUrl( username, deviceId );
    QNetworkReply* reply;
    QByteArray data;
    switch( type )
    {
    case Device::DESKTOP:
        data = JsonCreator::renameDeviceStringToJSON( caption, QLatin1String( "desktop" ) );
        break;
    case Device::LAPTOP:
        data = JsonCreator::renameDeviceStringToJSON( caption, QLatin1String( "laptop" ) );
        break;
    case Device::MOBILE:
        data = JsonCreator::renameDeviceStringToJSON( caption, QLatin1String( "mobile" ) );
        break;
    case Device::SERVER:
        data = JsonCreator::renameDeviceStringToJSON( caption, QLatin1String( "server" ) );
        break;
    case Device::OTHER:
        data = JsonCreator::renameDeviceStringToJSON( caption, QLatin1String( "other" ) );
        break;
    }
    reply = m_requestHandler.postRequest( data, requestUrl );
    return reply;
}

DeviceListPtr ApiRequestPrivate::listDevices( const QString& username )
{
    QString requestUrl = UrlBuilder::getDeviceListUrl( username );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    DeviceListPtr list( new DeviceList( reply ) );
    return list;
}

DeviceSyncResultPtr ApiRequestPrivate::deviceSynchronizationStatus ( const QString& username )
{
    QString requestUrl = UrlBuilder::getDeviceSynchronizationStatusUrl( username );
    QNetworkReply* reply;
    reply = m_requestHandler.authGetRequest( requestUrl );
    DeviceSyncResultPtr result( new DeviceSyncResult( reply ) );
    return result;
}

DeviceSyncResultPtr ApiRequestPrivate::setDeviceSynchronizationStatus(const QString& username, const QList< QStringList >& synchronize, const QList< QString >& stopSynchronize )
{
    QString requestUrl = UrlBuilder::getDeviceSynchronizationStatusUrl( username );
    QNetworkReply* reply;
    QByteArray data = JsonCreator::deviceSynchronizationListsToJSON( synchronize, stopSynchronize );
    reply = m_requestHandler.postRequest( data, requestUrl );
    DeviceSyncResultPtr result( new DeviceSyncResult( reply ) );
    return result;
}

ApiRequest::ApiRequest( const QString& username, const QString& password, QNetworkAccessManager* nam ) : d( new ApiRequestPrivate( username, password, nam ) )
{
}

ApiRequest::ApiRequest( QNetworkAccessManager* nam ) : d( new ApiRequestPrivate( nam ) )
{
}

ApiRequest::~ApiRequest()
{
    delete d;
}

QNetworkReply* ApiRequest::toplistOpml( uint count )
{
    return d->toplistOpml( count );
}

QNetworkReply* ApiRequest::searchOpml( const QString& query )
{
    return d->searchOpml( query );
}

QNetworkReply* ApiRequest::suggestionsOpml( uint count )
{
    return d->suggestionsOpml( count );
}

QNetworkReply* ApiRequest::downloadSubscriptionsOpml( const QString& username, const QString& device )
{
    return d->downloadSubscriptionsOpml( username, device );
}

QNetworkReply* ApiRequest::toplistTxt( uint count )
{
    return d->toplistTxt( count );
}

QNetworkReply* ApiRequest::searchTxt( const QString& query )
{
    return d->searchTxt( query );
}

QNetworkReply* ApiRequest::suggestionsTxt( uint count )
{
    return d->suggestionsTxt( count );
}

QNetworkReply* ApiRequest::downloadSubscriptionsTxt(const QString& username, const QString& device)
{
    return d->downloadSubscriptionsTxt( username, device );
}

QNetworkReply* ApiRequest::toplistXml ( uint count )
{
    return d->toplistXml( count );
}

QNetworkReply* ApiRequest::searchXml ( const QString& query )
{
    return d->searchXml( query );
}

PodcastListPtr ApiRequest::toplist( uint count )
{
    return d->toplist( count );
}

PodcastListPtr ApiRequest::search( const QString& query )
{
    return d->search( query );
}

PodcastListPtr ApiRequest::suggestions( uint count )
{
    return d->suggestions( count );
}

QNetworkReply* ApiRequest::downloadSubscriptionsJson(const QString& username, const QString& device)
{
    return d->downloadSubscriptionsJson( username, device );
}

PodcastListPtr ApiRequest::podcastsOfTag( uint count, const QString& tag )
{
    return d->podcastsOfTag( count, tag );
}

PodcastPtr ApiRequest::podcastData( const QUrl& podcasturl )
{
    return d->podcastData( podcasturl );
}

EpisodePtr ApiRequest::episodeData( const QUrl& podcasturl, const QUrl& episodeurl )
{
    return d->episodeData( podcasturl, episodeurl );
}

EpisodeListPtr ApiRequest::favoriteEpisodes( const QString& username )
{
    return d->favoriteEpisodes( username );
}

TagListPtr ApiRequest::topTags( uint count )
{
    return d->topTags( count );
}

AddRemoveResultPtr ApiRequest::addRemoveSubscriptions( const QString& username, const QString& device, const QList< QUrl >& add, const QList< QUrl >& remove )
{
    return d->addRemoveSubscriptions( username, device, add, remove );
}

SettingsPtr ApiRequest::accountSettings( const QString& username )
{
    return d->accountSettings( username );
}

SettingsPtr ApiRequest::deviceSettings( const QString& username, const QString& device )
{
    return d->deviceSettings( username, device );
}

SettingsPtr ApiRequest::podcastSettings( const QString& username, const QString& podcastUrl )
{
    return d->podcastSettings( username, podcastUrl );
}

SettingsPtr ApiRequest::episodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl )
{
    return d->episodeSettings( username, podcastUrl, episodeUrl );
}

SettingsPtr ApiRequest::setAccountSettings( const QString& username, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    return d->setAccountSettings( username, set, remove );
}

SettingsPtr ApiRequest::setDeviceSettings( const QString& username, const QString& device, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    return d->setDeviceSettings( username, device, set, remove );
}

SettingsPtr ApiRequest::setPodcastSettings( const QString& username, const QString& podcastUrl, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    return d->setPodcastSettings( username, podcastUrl, set, remove );
}

SettingsPtr ApiRequest::setEpisodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl, QMap<QString, QVariant >& set, const QList< QString >& remove )
{
    return d->setEpisodeSettings( username, podcastUrl, episodeUrl, set, remove );
}

DeviceUpdatesPtr ApiRequest::deviceUpdates( const QString& username, const QString& deviceId, qlonglong timestamp )
{
    return d->deviceUpdates( username, deviceId, timestamp );
}

EpisodeActionListPtr ApiRequest::episodeActions( const QString& username, const bool aggregated )
{
    return d->episodeActions( username, aggregated );
}

EpisodeActionListPtr ApiRequest::episodeActionsByPodcast( const QString& username, const QString& podcastUrl, const bool aggregated )
{
    return d->episodeActionsByPodcast( username, podcastUrl, aggregated );
}

EpisodeActionListPtr ApiRequest::episodeActionsByDevice( const QString& username, const QString& deviceId, const bool aggregated )
{
    return d->episodeActionsByDevice( username, deviceId, aggregated );
}

EpisodeActionListPtr ApiRequest::episodeActionsByTimestamp( const QString& username, const qulonglong since )
{
    return d->episodeActionsByTimestamp( username, since );
}

EpisodeActionListPtr ApiRequest::episodeActionsByPodcastAndTimestamp( const QString& username, const QString& podcastUrl, const qulonglong since )
{
    return d->episodeActionsByPodcastAndTimestamp( username, podcastUrl, since );
}

EpisodeActionListPtr ApiRequest::episodeActionsByDeviceAndTimestamp( const QString& username, const QString& deviceId, const qulonglong since )
{
    return d->episodeActionsByDeviceAndTimestamp( username, deviceId, since );
}

AddRemoveResultPtr ApiRequest::uploadEpisodeActions( const QString& username, const QList<EpisodeActionPtr>& episodeActions )
{
    return d->uploadEpisodeActions( username, episodeActions );
}

QNetworkReply* ApiRequest::renameDevice( const QString& username , const QString& deviceId, const QString& caption, Device::Type type )
{
    return d->renameDevice( username, deviceId, caption, type );
}

DeviceListPtr ApiRequest::listDevices( const QString& username )
{
    return d->listDevices( username );
}

DeviceSyncResultPtr ApiRequest::deviceSynchronizationStatus ( const QString& username )
{
    return d->deviceSynchronizationStatus( username );
}

DeviceSyncResultPtr ApiRequest::setDeviceSynchronizationStatus(const QString& username, const QList<QStringList>& synchronize, const QList<QString>& stopSynchronize )
{
    return d->setDeviceSynchronizationStatus( username, synchronize, stopSynchronize );
}
