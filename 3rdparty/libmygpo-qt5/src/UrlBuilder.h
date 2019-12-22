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

#ifndef LIBMYGPO_QT_URLBUILDER_H
#define LIBMYGPO_QT_URLBUILDER_H

#include <QString>

namespace mygpo
{
/**
* Helper class to generate request URL's.
* Helps to generate URL's for the gpodder requests.
* This class uses the singleton pattern, to retrieve a
* reference to the singleton object use the function instance().
*/

class UrlBuilder
{

public:

    enum Format
    {
        JSON,
        OPML,
        TEXT,
        XML
    };

    /**
    * @param i Any value between 1..100. If i <= 0 it will be set to 1.
    * @return Request URL to retrieve a list of the top 'i' podcasts.
    */
    static QString getToplistUrl( uint i, Format f = JSON );

    /**
     * @param i Any value between 1..100. If i <= 0 it will be set to 1.
     * @return Rquest URL to retrieve 'i' podcast suggestions.
     */
    static QString getSuggestionsUrl( uint i, Format f = JSON );

    /**
     * @param query The query to search in the podcasts name/descrption.
     * @return Request URL to retrieve podcasts related to the query.
     */
    static QString getPodcastSearchUrl( const QString& query, Format f = JSON );

    static QString getSubscriptionsUrl( const QString& username, const QString& device, Format f = JSON );
    /**
     * @param i Amount of tags. If i == 0 it will be set to 1.
     * @return Request URL to retrieve the 'i' most used tags.
     */
    static QString getTopTagsUrl( uint i );

    /**
     * @param i Amount of podcasts. If i == 0 it will be set to 1.
     * @return Request URL to retrieve the 'i' most-subscribed podcats that are tagged with tag.
     */
    static QString getPodcastsOfTagUrl( const QString& tag, uint i );

    /**
     * @param url The URL of the podcast
     * @return Request URL to retrieve information about the podcast with the given url.
     */
    static QString getPodcastDataUrl( const QString& url );

    /**
     * @param podcastUrl URL of the podcast
     * @param episodeUrl URL of the episode that belongs to the podcast-url
     * @return Request URL to retrieve information about the episode with the given episode-url.
     */
    static QString getEpisodeDataUrl( const QString& podcastUrl, const QString& episodeUrl );

    /**
     * @param username User name (gpodder.net). You need to be logged in with username.
     * @return Request URL to retrieve a list of all favorite episodes.
     */
    static QString getFavEpisodesUrl( const QString& username );

    /**
     * @param username User name (gpodder.net). You need to be logged in with username.
     * @param deviceId The id of the device.
     * @return Request URL to to update the subscription list for a given device.
     */
    static QString getAddRemoveSubUrl( const QString& username, const QString& deviceId );

    static QString getAccountSettingsUrl( const QString& username );

    static QString getDeviceSettingsUrl( const QString& username, const QString& deviceId );

    static QString getPodcastSettingsUrl( const QString& username, const QString& podcastUrl );

    static QString getEpisodeSettingsUrl( const QString& username, const QString& podcastUrl, const QString& episodeUrl );

    static QString getDeviceListUrl( const QString& username );

    static QString getRenameDeviceUrl( const QString& username, const QString& deviceId );

    static QString getDeviceUpdatesUrl( const QString& username, const QString& deviceId, qulonglong timestamp );

    static QString getEpisodeActionsUrl( const QString& username, bool aggregated );

    static QString getEpisodeActionsUrlByPodcast( const QString& username, const QString& podcastUrl, bool aggregated );

    static QString getEpisodeActionsUrlByDevice( const QString& username, const QString& deviceId, bool aggregated );

    static QString getEpisodeActionsUrlByTimestamp( const QString& username, qulonglong since );

    static QString getEpisodeActionsUrlByPodcastAndTimestamp( const QString& username, const QString& podcastUrl, qulonglong since );

    static QString getEpisodeActionsUrlByDeviceAndTimestamp( const QString& username, const QString& deviceId, qulonglong since );

    static QString getUploadEpisodeActionsUrl( const QString& username );

    static QString getDeviceSynchronizationStatusUrl( const QString& username );

private:
    UrlBuilder() {};
    UrlBuilder( const UrlBuilder& ) {};
    static const QString s_api2;
    static const QString s_api1;
};
}

#endif // LIBMYGPO_QT_URLBUILDER_H
