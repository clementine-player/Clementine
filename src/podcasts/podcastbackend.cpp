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

#include "podcastbackend.h"
#include "core/application.h"
#include "core/database.h"
#include "core/scopedtransaction.h"

#include <QMutexLocker>

PodcastBackend::PodcastBackend(Application* app, QObject* parent)
  : QObject(parent),
    app_(app),
    db_(app->database())
{
}

void PodcastBackend::Subscribe(Podcast* podcast) {
  // If this podcast is already in the database, do nothing
  if (podcast->is_valid()) {
    return;
  }

  // If there's an entry in the database with the same URL, take its data.
  Podcast existing_podcast = GetSubscriptionByUrl(podcast->url());
  if (existing_podcast.is_valid()) {
    *podcast = existing_podcast;
    return;
  }

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  ScopedTransaction t(&db);

  // Insert the podcast.
  QSqlQuery q("INSERT INTO podcasts (" + Podcast::kColumnSpec + ")"
              " VALUES (" + Podcast::kBindSpec + ")", db);
  podcast->BindToQuery(&q);

  q.exec();
  if (db_->CheckErrors(q))
    return;

  // Update the database ID.
  const int database_id = q.lastInsertId().toInt();
  podcast->set_database_id(database_id);

  // Update the IDs of any episodes.
  PodcastEpisodeList* episodes = podcast->mutable_episodes();
  for (PodcastEpisodeList::iterator it = episodes->begin() ; it != episodes->end() ; ++it) {
    it->set_podcast_database_id(database_id);
  }

  // Add those episodes to the database.
  AddEpisodes(episodes, &db);

  t.Commit();
}

void PodcastBackend::AddEpisodes(PodcastEpisodeList* episodes, QSqlDatabase* db) {
  QSqlQuery q("INSERT INTO podcast_episodes (" + PodcastEpisode::kColumnSpec + ")"
              " VALUES (" + PodcastEpisode::kBindSpec + ")", *db);

  for (PodcastEpisodeList::iterator it = episodes->begin() ; it != episodes->end() ; ++it) {
    it->BindToQuery(&q);
    q.exec();
    if (db_->CheckErrors(q))
      return;

    const int database_id = q.lastInsertId().toInt();
    it->set_database_id(database_id);
  }
}

#define SELECT_PODCAST_QUERY(where_clauses) \
  "SELECT p.ROWID, " + Podcast::kJoinSpec + "," \
  "       COUNT(e.ROWID), SUM(e.listened)" \
  " FROM podcasts AS p" \
  "   LEFT JOIN podcast_episodes AS e" \
  "   ON p.ROWID = e.podcast_id" \
  " " where_clauses \
  " GROUP BY p.ROWID" \
  " ORDER BY p.title"

namespace {
  void AddAggregatePodcastFields(const QSqlQuery& q, int column_count, Podcast* podcast) {
    const int episode_count = q.value(column_count + 1).toInt();
    const int listened_count = q.value(column_count + 2).toInt();

    podcast->set_extra("db:episode_count", episode_count);
    podcast->set_extra("db:unlistened_count", episode_count - listened_count);
  }
}

PodcastList PodcastBackend::GetAllSubscriptions() {
  PodcastList ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(SELECT_PODCAST_QUERY(""), db);

  q.exec();
  if (db_->CheckErrors(q))
    return ret;

  static const int kPodcastColumnCount = Podcast::kColumns.count();

  while (q.next()) {
    Podcast podcast;
    podcast.InitFromQuery(q);

    AddAggregatePodcastFields(q, kPodcastColumnCount, &podcast);

    ret << podcast;
  }

  return ret;
}

Podcast PodcastBackend::GetSubscriptionById(int id) {
  Podcast ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(SELECT_PODCAST_QUERY("WHERE ROWID = :id"), db);
  q.bindValue(":id", id);
  q.exec();
  if (!db_->CheckErrors(q) && q.next()) {
    ret.InitFromQuery(q);
    AddAggregatePodcastFields(q, Podcast::kColumns.count(), &ret);
  }

  return ret;
}

Podcast PodcastBackend::GetSubscriptionByUrl(const QUrl& url) {
  Podcast ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(SELECT_PODCAST_QUERY("WHERE p.url = :url"), db);
  q.bindValue(":url", url.toEncoded());
  q.exec();
  if (!db_->CheckErrors(q) && q.next()) {
    ret.InitFromQuery(q);
    AddAggregatePodcastFields(q, Podcast::kColumns.count(), &ret);
  }

  return ret;
}
