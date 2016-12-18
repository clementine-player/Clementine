/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof A. Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include <QMutexLocker>

#include "core/application.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/scopedtransaction.h"

PodcastBackend::PodcastBackend(Application* app, QObject* parent)
    : QObject(parent), app_(app), db_(app->database()) {}

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
  QSqlQuery q("INSERT INTO podcasts (" + Podcast::kColumnSpec +
                  ")"
                  " VALUES (" +
                  Podcast::kBindSpec + ")",
              db);
  podcast->BindToQuery(&q);

  q.exec();
  if (db_->CheckErrors(q)) return;

  // Update the database ID.
  const int database_id = q.lastInsertId().toInt();
  podcast->set_database_id(database_id);

  // Update the IDs of any episodes.
  PodcastEpisodeList* episodes = podcast->mutable_episodes();
  for (auto it = episodes->begin(); it != episodes->end(); ++it) {
    it->set_podcast_database_id(database_id);
  }

  // Add those episodes to the database.
  AddEpisodes(episodes, &db);

  t.Commit();

  emit SubscriptionAdded(*podcast);
}

void PodcastBackend::Unsubscribe(const Podcast& podcast) {
  // If this podcast is not already in the database, do nothing
  if (!podcast.is_valid()) {
    return;
  }

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  ScopedTransaction t(&db);

  // Remove the podcast.
  QSqlQuery q("DELETE FROM podcasts WHERE ROWID = :id", db);
  q.bindValue(":id", podcast.database_id());
  q.exec();
  if (db_->CheckErrors(q)) return;

  // Remove all episodes in the podcast
  q = QSqlQuery("DELETE FROM podcast_episodes WHERE podcast_id = :id", db);
  q.bindValue(":id", podcast.database_id());
  q.exec();
  if (db_->CheckErrors(q)) return;

  t.Commit();

  emit SubscriptionRemoved(podcast);
}

void PodcastBackend::AddEpisodes(PodcastEpisodeList* episodes,
                                 QSqlDatabase* db) {
  QSqlQuery q("INSERT INTO podcast_episodes (" + PodcastEpisode::kColumnSpec +
                  ")"
                  " VALUES (" +
                  PodcastEpisode::kBindSpec + ")",
              *db);

  for (auto it = episodes->begin(); it != episodes->end(); ++it) {
    it->BindToQuery(&q);
    q.exec();
    if (db_->CheckErrors(q)) continue;

    const int database_id = q.lastInsertId().toInt();
    it->set_database_id(database_id);
  }
}

void PodcastBackend::AddEpisodes(PodcastEpisodeList* episodes) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  ScopedTransaction t(&db);

  AddEpisodes(episodes, &db);
  t.Commit();

  emit EpisodesAdded(*episodes);
}

void PodcastBackend::UpdateEpisodes(const PodcastEpisodeList& episodes) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());
  ScopedTransaction t(&db);

  QSqlQuery q(
      "UPDATE podcast_episodes"
      " SET listened = :listened,"
      "     listened_date = :listened_date,"
      "     downloaded = :downloaded,"
      "     local_url = :local_url"
      " WHERE ROWID = :id",
      db);

  for (const PodcastEpisode& episode : episodes) {
    q.bindValue(":listened", episode.listened());
    q.bindValue(":listened_date", episode.listened_date().toTime_t());
    q.bindValue(":downloaded", episode.downloaded());
    q.bindValue(":local_url", episode.local_url().toEncoded());
    q.bindValue(":id", episode.database_id());
    q.exec();
    db_->CheckErrors(q);
  }

  t.Commit();

  emit EpisodesUpdated(episodes);
}

PodcastList PodcastBackend::GetAllSubscriptions() {
  PodcastList ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + Podcast::kColumnSpec + " FROM podcasts", db);
  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    Podcast podcast;
    podcast.InitFromQuery(q);
    ret << podcast;
  }

  return ret;
}

Podcast PodcastBackend::GetSubscriptionById(int id) {
  Podcast ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + Podcast::kColumnSpec +
                  " FROM podcasts"
                  " WHERE ROWID = :id",
              db);
  q.bindValue(":id", id);
  q.exec();
  if (!db_->CheckErrors(q) && q.next()) {
    ret.InitFromQuery(q);
  }

  return ret;
}

Podcast PodcastBackend::GetSubscriptionByUrl(const QUrl& url) {
  Podcast ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + Podcast::kColumnSpec +
                  " FROM podcasts"
                  " WHERE url = :url",
              db);
  q.bindValue(":url", url.toEncoded());
  q.exec();
  if (!db_->CheckErrors(q) && q.next()) {
    ret.InitFromQuery(q);
  }

  return ret;
}

PodcastEpisodeList PodcastBackend::GetEpisodes(int podcast_id) {
  PodcastEpisodeList ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + PodcastEpisode::kColumnSpec +
                  " FROM podcast_episodes"
                  " WHERE podcast_id = :id"
                  " ORDER BY publication_date DESC",
              db);
  q.bindValue(":id", podcast_id);
  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    PodcastEpisode episode;
    episode.InitFromQuery(q);
    ret << episode;
  }

  return ret;
}

PodcastEpisode PodcastBackend::GetEpisodeById(int id) {
  PodcastEpisode ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + PodcastEpisode::kColumnSpec +
                  " FROM podcast_episodes"
                  " WHERE ROWID = :id",
              db);
  q.bindValue(":id", id);
  q.exec();
  if (!db_->CheckErrors(q) && q.next()) {
    ret.InitFromQuery(q);
  }

  return ret;
}

PodcastEpisode PodcastBackend::GetEpisodeByUrl(const QUrl& url) {
  PodcastEpisode ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + PodcastEpisode::kColumnSpec +
                  " FROM podcast_episodes"
                  " WHERE url = :url",
              db);
  q.bindValue(":url", url.toEncoded());
  q.exec();
  if (!db_->CheckErrors(q) && q.next()) {
    ret.InitFromQuery(q);
  }

  return ret;
}

PodcastEpisode PodcastBackend::GetEpisodeByUrlOrLocalUrl(const QUrl& url) {
  PodcastEpisode ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + PodcastEpisode::kColumnSpec +
                  " FROM podcast_episodes"
                  " WHERE url = :url"
                  "    OR local_url = :url",
              db);
  q.bindValue(":url", url.toEncoded());
  q.exec();
  if (!db_->CheckErrors(q) && q.next()) {
    ret.InitFromQuery(q);
  }

  return ret;
}

PodcastEpisodeList PodcastBackend::GetOldDownloadedEpisodes(
    const QDateTime& max_listened_date) {
  PodcastEpisodeList ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + PodcastEpisode::kColumnSpec +
                  " FROM podcast_episodes"
                  " WHERE downloaded = 'true'"
                  "   AND listened_date <= :max_listened_date",
              db);
  q.bindValue(":max_listened_date", max_listened_date.toTime_t());
  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    PodcastEpisode episode;
    episode.InitFromQuery(q);
    ret << episode;
  }

  return ret;
}

PodcastEpisode PodcastBackend::GetOldestDownloadedListenedEpisode() {
  PodcastEpisode ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + PodcastEpisode::kColumnSpec +
                  " FROM podcast_episodes"
                  " WHERE downloaded = 'true'"
                  " AND listened = 'true'"
                  " ORDER BY listened_date ASC",
              db);
  q.exec();
  if (db_->CheckErrors(q)) return ret;
  q.next();
  ret.InitFromQuery(q);

  return ret;
}

PodcastEpisodeList PodcastBackend::GetNewDownloadedEpisodes() {
  PodcastEpisodeList ret;

  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q("SELECT ROWID, " + PodcastEpisode::kColumnSpec +
                  " FROM podcast_episodes"
                  " WHERE downloaded = 'true'"
                  "   AND listened = 'false'",
              db);
  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    PodcastEpisode episode;
    episode.InitFromQuery(q);
    ret << episode;
  }

  return ret;
}
