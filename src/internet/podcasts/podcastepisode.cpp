/* This file is part of Clementine.
   Copyright 2012-2014, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof A. Sobiecki <sobkas@gmail.com>

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

#include "podcastepisode.h"

#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>

#include "core/logging.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "podcast.h"

const QStringList PodcastEpisode::kColumns = QStringList() << "podcast_id"
                                                           << "title"
                                                           << "description"
                                                           << "author"
                                                           << "publication_date"
                                                           << "duration_secs"
                                                           << "url"
                                                           << "listened"
                                                           << "listened_date"
                                                           << "downloaded"
                                                           << "local_url"
                                                           << "extra";

const QString PodcastEpisode::kColumnSpec = PodcastEpisode::kColumns.join(", ");
const QString PodcastEpisode::kJoinSpec =
    Utilities::Prepend("e.", PodcastEpisode::kColumns).join(", ");
const QString PodcastEpisode::kBindSpec =
    Utilities::Prepend(":", PodcastEpisode::kColumns).join(", ");
const QString PodcastEpisode::kUpdateSpec =
    Utilities::Updateify(PodcastEpisode::kColumns).join(", ");

struct PodcastEpisode::Private : public QSharedData {
  Private();

  int database_id_;
  int podcast_database_id_;

  QString title_;
  QString description_;
  QString author_;
  QDateTime publication_date_;
  int duration_secs_;
  QUrl url_;

  bool listened_;
  QDateTime listened_date_;

  bool downloaded_;
  QUrl local_url_;

  QVariantMap extra_;
};

PodcastEpisode::Private::Private()
    : database_id_(-1),
      podcast_database_id_(-1),
      duration_secs_(-1),
      listened_(false),
      downloaded_(false) {}

PodcastEpisode::PodcastEpisode() : d(new Private) {}

PodcastEpisode::PodcastEpisode(const PodcastEpisode& other) : d(other.d) {}

PodcastEpisode::~PodcastEpisode() {}

PodcastEpisode& PodcastEpisode::operator=(const PodcastEpisode& other) {
  d = other.d;
  return *this;
}

int PodcastEpisode::database_id() const { return d->database_id_; }
int PodcastEpisode::podcast_database_id() const {
  return d->podcast_database_id_;
}
const QString& PodcastEpisode::title() const { return d->title_; }
const QString& PodcastEpisode::description() const { return d->description_; }
const QString& PodcastEpisode::author() const { return d->author_; }
const QDateTime& PodcastEpisode::publication_date() const {
  return d->publication_date_;
}
int PodcastEpisode::duration_secs() const { return d->duration_secs_; }
const QUrl& PodcastEpisode::url() const { return d->url_; }
bool PodcastEpisode::listened() const { return d->listened_; }
const QDateTime& PodcastEpisode::listened_date() const {
  return d->listened_date_;
}
bool PodcastEpisode::downloaded() const { return d->downloaded_; }
const QUrl& PodcastEpisode::local_url() const { return d->local_url_; }
const QVariantMap& PodcastEpisode::extra() const { return d->extra_; }
QVariant PodcastEpisode::extra(const QString& key) const {
  return d->extra_[key];
}

void PodcastEpisode::set_database_id(int v) { d->database_id_ = v; }
void PodcastEpisode::set_podcast_database_id(int v) {
  d->podcast_database_id_ = v;
}
void PodcastEpisode::set_title(const QString& v) { d->title_ = v; }
void PodcastEpisode::set_description(const QString& v) { d->description_ = v; }
void PodcastEpisode::set_author(const QString& v) { d->author_ = v; }
void PodcastEpisode::set_publication_date(const QDateTime& v) {
  d->publication_date_ = v;
}
void PodcastEpisode::set_duration_secs(int v) { d->duration_secs_ = v; }
void PodcastEpisode::set_url(const QUrl& v) { d->url_ = v; }
void PodcastEpisode::set_listened(bool v) { d->listened_ = v; }
void PodcastEpisode::set_listened_date(const QDateTime& v) {
  d->listened_date_ = v;
}
void PodcastEpisode::set_downloaded(bool v) { d->downloaded_ = v; }
void PodcastEpisode::set_local_url(const QUrl& v) { d->local_url_ = v; }
void PodcastEpisode::set_extra(const QVariantMap& v) { d->extra_ = v; }
void PodcastEpisode::set_extra(const QString& key, const QVariant& value) {
  d->extra_[key] = value;
}

void PodcastEpisode::InitFromQuery(const QSqlQuery& query) {
  d->database_id_ = query.value(0).toInt();
  d->podcast_database_id_ = query.value(1).toInt();
  d->title_ = query.value(2).toString();
  d->description_ = query.value(3).toString();
  d->author_ = query.value(4).toString();
  d->publication_date_ = QDateTime::fromTime_t(query.value(5).toUInt());
  d->duration_secs_ = query.value(6).toInt();
  d->url_ = QUrl::fromEncoded(query.value(7).toByteArray());
  d->listened_ = query.value(8).toBool();

  // After setting QDateTime to invalid state, it's saved into database as
  // time_t, when this number std::numeric_limits<unsigned int>::max()
  // (4294967295) is read back from database, it creates a valid QDateTime. So
  // to make it behave consistently, this change is needed.
  if (query.value(9).toUInt() == std::numeric_limits<unsigned int>::max()) {
    d->listened_date_ = QDateTime();
  } else {
    d->listened_date_ = QDateTime::fromTime_t(query.value(9).toUInt());
  }

  d->downloaded_ = query.value(10).toBool();
  d->local_url_ = QUrl::fromEncoded(query.value(11).toByteArray());

  QDataStream extra_stream(query.value(12).toByteArray());
  extra_stream >> d->extra_;
}

void PodcastEpisode::BindToQuery(QSqlQuery* query) const {
  query->bindValue(":podcast_id", d->podcast_database_id_);
  query->bindValue(":title", d->title_);
  query->bindValue(":description", d->description_);
  query->bindValue(":author", d->author_);
  query->bindValue(":publication_date", d->publication_date_.toTime_t());
  query->bindValue(":duration_secs", d->duration_secs_);
  query->bindValue(":url", d->url_.toEncoded());
  query->bindValue(":listened", d->listened_);
  query->bindValue(":listened_date", d->listened_date_.toTime_t());
  query->bindValue(":downloaded", d->downloaded_);
  query->bindValue(":local_url", d->local_url_.toEncoded());

  QByteArray extra;
  QDataStream extra_stream(&extra, QIODevice::WriteOnly);
  extra_stream << d->extra_;

  query->bindValue(":extra", extra);
}

Song PodcastEpisode::ToSong(const Podcast& podcast) const {
  Song ret;
  ret.set_valid(true);
  ret.set_title(title().simplified());
  ret.set_artist(author().simplified());
  ret.set_length_nanosec(kNsecPerSec * duration_secs());
  ret.set_year(publication_date().date().year());
  ret.set_comment(description());
  ret.set_id(database_id());
  ret.set_ctime(publication_date().toTime_t());
  ret.set_genre(QString("Podcast"));
  ret.set_genre_id3(186);

  if (listened() && listened_date().isValid()) {
    ret.set_mtime(listened_date().toTime_t());
  } else {
    ret.set_mtime(publication_date().toTime_t());
  }

  if (ret.length_nanosec() < 0) {
    ret.set_length_nanosec(-1);
  }

  if (downloaded() && QFile::exists(local_url().toLocalFile())) {
    ret.set_url(local_url());
  } else {
    ret.set_url(url());
  }

  ret.set_basefilename(QFileInfo(ret.url().path()).fileName());

  // Use information from the podcast if it's set
  if (podcast.is_valid()) {
    ret.set_album(podcast.title().simplified());
    ret.set_art_automatic(podcast.ImageUrlLarge().toString());

    if (author().isEmpty()) ret.set_artist(podcast.title().simplified());
  }
  return ret;
}
