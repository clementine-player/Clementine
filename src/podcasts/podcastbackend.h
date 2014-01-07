/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PODCASTBACKEND_H
#define PODCASTBACKEND_H

#include <QObject>

#include "podcast.h"

class Application;
class Database;

class PodcastBackend : public QObject {
  Q_OBJECT

public:
  PodcastBackend(Application* app, QObject* parent = 0);

  // Adds the podcast and any included Episodes to the database.  Updates the
  // podcast with a database ID.  If this podcast already has an ID set, this
  // function does nothing.  If a podcast with this URL already exists in the
  // database, this function just updates the ID field in the provided podcast.
  void Subscribe(Podcast* podcast);

  // Removes the Podcast with the given ID from the database.  Also removes any
  // episodes associated with this podcast.
  void Unsubscribe(const Podcast& podcast);

  // Returns a list of all the subscribed podcasts.  For efficiency the Podcast
  // objects returned won't contain any PodcastEpisode objects - get them
  // separately if you want them.
  PodcastList GetAllSubscriptions();
  Podcast GetSubscriptionById(int id);
  Podcast GetSubscriptionByUrl(const QUrl& url);

  // Returns podcast episodes that match various keys.  All these queries are
  // indexed.
  PodcastEpisodeList GetEpisodes(int podcast_id);
  PodcastEpisode GetEpisodeById(int id);
  PodcastEpisode GetEpisodeByUrl(const QUrl& url);
  PodcastEpisode GetEpisodeByUrlOrLocalUrl(const QUrl& url);

  // Returns a list of episodes that have local data (downloaded=true) but were
  // last listened to before the given QDateTime.  This query is NOT indexed so
  // it involves a full search of the table.
  PodcastEpisodeList GetOldDownloadedEpisodes(const QDateTime& max_listened_date);
  PodcastEpisodeList GetNewDownloadedEpisodes();

  // Adds episodes to the database.  Every episode must have a valid
  // podcast_database_id set already.
  void AddEpisodes(PodcastEpisodeList* episodes);

  // Updates the editable fields (listened, listened_date, downloaded, and
  // local_url) on episodes that must already exist in the database.
  void UpdateEpisodes(const PodcastEpisodeList& episodes);
  
signals:
  void SubscriptionAdded(const Podcast& podcast);
  void SubscriptionRemoved(const Podcast& podcast);

  // Emitted when episodes are added to a subscription that *already exists*.
  void EpisodesAdded(const QList<PodcastEpisode>& episodes);

  // Emitted when existing episodes are updated.
  void EpisodesUpdated(const QList<PodcastEpisode>& episodes);

private:
  // Adds each episode to the database, setting their IDs after inserting each
  // one.
  void AddEpisodes(PodcastEpisodeList* episodes, QSqlDatabase* db);

private:
  Application* app_;
  Database* db_;
};

#endif // PODCASTBACKEND_H
