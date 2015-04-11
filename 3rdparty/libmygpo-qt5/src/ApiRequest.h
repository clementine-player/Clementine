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

#ifndef LIBMYGPO_QT_APIREQUEST_H
#define LIBMYGPO_QT_APIREQUEST_H

#include "mygpo_export.h"
#include "AddRemoveResult.h"
#include "EpisodeList.h"
#include "EpisodeActionList.h"
#include "PodcastList.h"
#include "TagList.h"
#include "Settings.h"
#include "DeviceUpdates.h"
#include "DeviceList.h"
#include "DeviceSyncResult.h"

class QByteArray;
class QString;

namespace mygpo
{

class ApiRequestPrivate;

/**
 * This Class is the Frontend of libmygpo-qt.
 * Methods from this Class map the Web API of gpodder.net
 * and return the Results of the Requests.
 * Web API Documentation can be found here: http://wiki.gpodder.org/wiki/Web_Services/API_2
 */

class MYGPO_EXPORT ApiRequest
{
public:

    ApiRequest( const QString& username, const QString& password, QNetworkAccessManager* nam );
    ApiRequest( QNetworkAccessManager* nam );
    ~ApiRequest( );

    //SIMPLE API

    /**
     * Returns the OPML Result for the Simple API Call "Downloading Podcast Toplists"
     * @param count The number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* toplistOpml( uint count );

    /**
     * Returns the OPML Result for the Simple API Call "Searching for Podcasts"
     * @param query The String you want to search for
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* searchOpml( const QString& query );

    /**
     * Returns the OPML Result for the Simple API Call "Downloading podcast suggestions"
     * Requires Authentication
     * @param count The maximum number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* suggestionsOpml( uint count );

    QNetworkReply* downloadSubscriptionsOpml( const QString& username, const QString& device = QString() );

    /**
     * Returns the TXT Result for the Simple API Call "Downloading Podcast Toplists"
     * @param count The number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* toplistTxt( uint count );

    /**
     * Returns the TXT Result for the Simple API Call "Searching for Podcasts"
     * @param query The String you want to search for
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* searchTxt( const QString& query );

    /**
     * Returns the TXT Result for the Simple API Call "Downloading podcast suggestions"
     * Requires Authentication
     * @param count The maximum number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* suggestionsTxt( uint count );

    QNetworkReply* downloadSubscriptionsTxt( const QString& username, const QString& device = QString() );

    /**
     * Returns the TXT Result for the Simple API Call "Downloading Podcast Toplists"
     * @param count The number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* toplistXml( uint count );

    /**
     * Returns the XML Result for the Simple API Call "Searching for Podcasts"
     * @param query The String you want to search for
     * @return A Pointer to a QNetworkReply which receives network signals and will contain the data
     *
     */
    QNetworkReply* searchXml( const QString& query );

    /**
     * Returns the Result for the Simple API Call "Downloading Podcast Toplists"
     * @param count The number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @return List of Podcast Objects containing the Data from gPodder
     *
     */
    PodcastListPtr toplist( uint count );

    /**
     * Returns the Result for the Simple API Call "Searching for Podcasts"
     * @param query The String you want to search for
     * @return List of Podcast Objects containing the Data from gPodder
     *
     */
    PodcastListPtr search( const QString& query );

    /**
     * Returns the Result for the Simple API Call "Downloading podcast suggestions"
     * Requires Authentication
     * @param count The maximum number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @return List of Podcast Objects containing the Data from gPodder
     *
     */
    PodcastListPtr suggestions( uint count );

    QNetworkReply* downloadSubscriptionsJson( const QString& username, const QString& device = QString() );

    //ADVANCED API

    /**
     * Returns the Result for the Advanced API Call "Retrieving Podcasts of a Tag"
     * @param query The number of Podcasts that should be returned - will be set to to 100 if > 100 or < 1
     * @param tag The Tag for which Podcasts should be retrieved
     * @return List of Podcast Objects containing the Data from gPodder
     *
     */
    PodcastListPtr podcastsOfTag( uint count, const QString& tag );

    /**
     * Returns the Result for the Advanced API Call "Retrieving Podcast Data"
     * @param podcasturl Url of the Podcast for which Data should be retrieved
     * @return Podcast Object containing the Data from gPodder
     *
     */
    PodcastPtr podcastData( const QUrl& podcasturl );

    /**
     * Returns the Result for the Advanced API Call "Retrieving Episode Data"
     * @param podcasturl Url of the Podcast that contains the Episode
     * @param episodeurl Url of the Episode Data for which Data should be retrieved
     * @return Episode Object containing the Data from gPodder
     *
     */
    EpisodePtr episodeData( const QUrl& podcasturl, const QUrl& episodeurl );

    /**
     * Returns the Result for the Advanced API Call "Listing Favorite Episodes"
     * @param username The User whose Favorite Episodes should be retrieved
     * @return List of Episode Objects containing the Data from gPodder
     *
     */
    EpisodeListPtr favoriteEpisodes( const QString& username );

    /**
     * Returns the Result for the Advanced API Call "Retrieving Top Tags"
     * @param count The number of Tags that should be returned - will be set to to 100 if > 100 or < 1
     * @return List of Tag Objects containing the Data from gPodder
     *
     */
    TagListPtr topTags( uint count );

    /**
     * Uploads Data & returns the Result for the Advanced API Call "Add/remove subscriptions"
     * Requires Authentication.
     * @param username User for which this API Call should be executed
     * @param device gPodder Device for which this API Call should be executed
     * @param add URLs of Podcasts that should be added to the Subscriptions of the User
     * @param remove URLs of Podcasts that should be removed from the Subscriptions of the User
     *
     */
    AddRemoveResultPtr addRemoveSubscriptions( const QString& username, const QString& device, const QList< QUrl >& add, const QList< QUrl >& remove );

    /**
     * Retrieve settings which are attached to an account.
     * @param username Username of the targeted account
     * @return Received settings as key-value-pairs
     *
     */
    SettingsPtr accountSettings( const QString& username );

    /**
     * Retrieve settings which are attached to a device.
     * @param username Username of the account which owns the device
     * @param device Name of the targeted device
     * @return Received settings as key-value-pairs
     *
     */
    SettingsPtr deviceSettings( const QString& username, const QString& device );

    /**
     * Retrieve settings which are attached to a podcast.
     * @param username Username of the account which owns the podcast
     * @param podcastUrl Url which identifies the targeted podcast
     * @return Received settings as key-value-pairs
     *
     */
    SettingsPtr podcastSettings( const QString& username, const QString& podcastUrl );

    /**
     * Retrieve settings which are attached to an episode.
     * @param username Username of the account which owns the episode
     * @param podcastUrl Url as String which identifies the podcast to which the episode belongs to
     * @param episodeUrl Url as String which identifies the targeted episode
     * @return Received settings as key-value-pairs
     *
     */
    SettingsPtr episodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl );

    /**
     * Set and or remove settings which are attached to an account.
     * @param username Username of the targeted account
     * @param set A set of settings as key-value-pairs which shall be set
     * @param set A set of exisiting settings as key-value-pairs which shall be removed
     * @return All settings as key-value-pairs which are stored after the update
     *
     */
    SettingsPtr setAccountSettings( const QString& username, QMap<QString, QVariant >& set, const QList<QString>& remove );

    /**
     * Set and or remove settings which are attached to a device.
     * @param username Username of the account which owns the device
     * @param device Name of the targeted device
     * @param set A set of settings as key-value-pairs which shall be set
     * @param set A set of exisiting settings as key-value-pairs which shall be removed
     * @return All settings as key-value-pairs which are stored after the update
     *
     */
    SettingsPtr setDeviceSettings( const QString& username, const QString& device, QMap<QString, QVariant >& set, const QList<QString>& remove );

    /**
     * Set and or remove settings which are attached to a podcast.
     * @param username Username of the account which owns the podcast
     * @param podcastUrl Url which identifies the targeted podcast
     * @param set A set of settings as key-value-pairs which shall be set
     * @param set A set of exisiting settings as key-value-pairs which shall be removed
     * @return All settings as key-value-pairs which are stored after the update
     *
     */
    SettingsPtr setPodcastSettings( const QString& username, const QString& podcastUrl, QMap<QString, QVariant >& set, const QList<QString>& remove );

    /**
     * Set and or remove settings which are attached to an episode.
     * @param username Username of the account which owns the episode
     * @param podcastUrl Url as String which identifies the podcast to which the episode belongs to
     * @param episodeUrl Url as String which identifies the targeted episode
     * @param set A set of settings as key-value-pairs which shall be set
     * @param set A set of exisiting settings as key-value-pairs which shall be removed
     * @return All settings as key-value-pairs which are stored after the update
     *
     */
    SettingsPtr setEpisodeSettings( const QString& username, const QString& podcastUrl, const QString& episodeUrl, QMap<QString, QVariant >& set, const QList<QString>& remove );

    /**
     * Retrieve episode and subscription updates for a given device.
     * @param username Username of the account which owns the device
     * @param deviceId Id of the targeted device
     * @param timestamp A date in milliseconds, All changes since this timestamp will be retrieved
     * @return A DeviceUpdatesPtr which accesses:
     *    - a list of subscriptions to be added, with URL, title and descriptions
     *    - a list of URLs to be unsubscribed
     *    - a list of updated episodes
     *
     */
    DeviceUpdatesPtr deviceUpdates( const QString& username, const QString& deviceId, qlonglong timestamp );

    /**
     * Sets a new name and type for a device identified by a given ID
     * @param username Username of the account which owns the device
     * @param deviceId The id of the targeted device
     * @param caption The new name of the device
     * @param type The new type of the device
     * @return A Pointer to a QNetworkReply which receives network signals
     *
     */
    QNetworkReply* renameDevice( const QString& username, const QString& deviceId, const QString& caption, Device::Type type );

    /**
     * Returns the list of devices that belong to a user.
     * @param username Username of the targeted user
     * @return List of devices
     *
     */
    DeviceListPtr listDevices( const QString& username );

    /**
     * Download episode actions for a given username.
     * @param Username of the targeted user
     * @param aggregated If aggregated is set to true, only the latest episode action will be returned
     * @return List of all episode actions of the user
     *
     */
    EpisodeActionListPtr episodeActions( const QString& username, const bool aggregated = false );

    /**
     * Download episode actions for a given podcast.
     * @param username Username of the account which owns the podcast
     * @param podcastUrl Url which identifies the targeted podcast
     * @param aggregated If aggregated is set to true, only the latest episode action will be returned
     * @return List of all episode actions for the given podcast
     *
     */
    EpisodeActionListPtr episodeActionsByPodcast( const QString& username, const QString& podcastUrl, const bool aggregated = false );

    /**
     * Download episode actions for a given device.
     * @param username Username of the account which owns the device
     * @param deviceId The Id of the targeted device
     * @param aggregated If aggregated is set to true, only the latest episode action will be returned
     * @return List of all episode actions for the given device
     *
     */
    EpisodeActionListPtr episodeActionsByDevice( const QString& username, const QString& deviceId, const bool aggregated = false );

    /**
     * Download episode actions for a given username since a given timestamp.
     * @param Username of the targeted user
     * @param since Timestamp in milliseconds, Episode Actions since this time will be retrieved
     * @return List of all new episode actions since the given timestamp
     *
     */
    EpisodeActionListPtr episodeActionsByTimestamp( const QString& username, const qulonglong since );

    /**
     * Download episode actions for a given podcast since a given timestamp.
     * @param username Username of the account which owns the podcast
     * @param podcastUrl Url which identifies the targeted podcast
     * @param since Timestamp in milliseconds, Episode Actions since this time will be retrieved
     * @return List of all new episode actions since the given timestamp
     *
     */
    EpisodeActionListPtr episodeActionsByPodcastAndTimestamp( const QString& username, const QString& podcastUrl, const qulonglong since );

    /**
     * Download episode actions for a given device since a given timestamp.
     * @param username Username of the account which owns the device
     * @param deviceId The Id of the targeted device
     * @param since Timestamp in milliseconds, Episode Actions since this time will be retrieved
     * @return List of all new episode actions since the given timestamp
     *
     */
    EpisodeActionListPtr episodeActionsByDeviceAndTimestamp( const QString& username, const QString& deviceId, const qulonglong since );

    /**
     * Upload episode actions
     * @param episodeActions The list of episode actions which shall be uploaded
     * @return An AddRemoveResultPtr which contains information about the updated Urls
     *
     */
    AddRemoveResultPtr uploadEpisodeActions( const QString& username, const QList<EpisodeActionPtr>& episodeActions );

    DeviceSyncResultPtr deviceSynchronizationStatus( const QString& username );

    DeviceSyncResultPtr setDeviceSynchronizationStatus( const QString& username, const QList<QStringList>& synchronize, const QList<QString>& stopSynchronize );

private:
    ApiRequestPrivate* const d;
};

}

#endif
