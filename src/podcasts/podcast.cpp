/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
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

#include "podcast.h"

#include <QDataStream>
#include <QDateTime>
#include <Podcast.h>

#include "core/utilities.h"

const QStringList Podcast::kColumns = QStringList() << "url"
                                                    << "title"
                                                    << "description"
                                                    << "copyright"
                                                    << "link"
                                                    << "image_url_large"
                                                    << "image_url_small"
                                                    << "author"
                                                    << "owner_name"
                                                    << "owner_email"
                                                    << "last_updated"
                                                    << "last_update_error"
                                                    << "extra";

const QString Podcast::kColumnSpec = Podcast::kColumns.join(", ");
const QString Podcast::kJoinSpec =
    Utilities::Prepend("p.", Podcast::kColumns).join(", ");
const QString Podcast::kBindSpec =
    Utilities::Prepend(":", Podcast::kColumns).join(", ");
const QString Podcast::kUpdateSpec =
    Utilities::Updateify(Podcast::kColumns).join(", ");

struct Podcast::Private : public QSharedData {
  Private();

  int database_id_;
  QUrl url_;

  QString title_;
  QString description_;
  QString copyright_;
  QUrl link_;
  QUrl image_url_large_;
  QUrl image_url_small_;

  // iTunes extensions
  QString author_;
  QString owner_name_;
  QString owner_email_;

  QDateTime last_updated_;
  QString last_update_error_;

  QVariantMap extra_;

  // These are stored in a different table
  PodcastEpisodeList episodes_;
};

Podcast::Private::Private() : database_id_(-1) {}

Podcast::Podcast() : d(new Private) {}

Podcast::Podcast(const Podcast& other) : d(other.d) {}

Podcast::~Podcast() {}

Podcast& Podcast::operator=(const Podcast& other) {
  d = other.d;
  return *this;
}

int Podcast::database_id() const { return d->database_id_; }
const QUrl& Podcast::url() const { return d->url_; }
const QString& Podcast::title() const { return d->title_; }
const QString& Podcast::description() const { return d->description_; }
const QString& Podcast::copyright() const { return d->copyright_; }
const QUrl& Podcast::link() const { return d->link_; }
const QUrl& Podcast::image_url_large() const { return d->image_url_large_; }
const QUrl& Podcast::image_url_small() const { return d->image_url_small_; }
const QString& Podcast::author() const { return d->author_; }
const QString& Podcast::owner_name() const { return d->owner_name_; }
const QString& Podcast::owner_email() const { return d->owner_email_; }
const QDateTime& Podcast::last_updated() const { return d->last_updated_; }
const QString& Podcast::last_update_error() const {
  return d->last_update_error_;
}
const QVariantMap& Podcast::extra() const { return d->extra_; }
QVariant Podcast::extra(const QString& key) const { return d->extra_[key]; }

void Podcast::set_database_id(int v) { d->database_id_ = v; }
void Podcast::set_url(const QUrl& v) { d->url_ = v; }
void Podcast::set_title(const QString& v) { d->title_ = v; }
void Podcast::set_description(const QString& v) { d->description_ = v; }
void Podcast::set_copyright(const QString& v) { d->copyright_ = v; }
void Podcast::set_link(const QUrl& v) { d->link_ = v; }
void Podcast::set_image_url_large(const QUrl& v) { d->image_url_large_ = v; }
void Podcast::set_image_url_small(const QUrl& v) { d->image_url_small_ = v; }
void Podcast::set_author(const QString& v) { d->author_ = v; }
void Podcast::set_owner_name(const QString& v) { d->owner_name_ = v; }
void Podcast::set_owner_email(const QString& v) { d->owner_email_ = v; }
void Podcast::set_last_updated(const QDateTime& v) { d->last_updated_ = v; }
void Podcast::set_last_update_error(const QString& v) {
  d->last_update_error_ = v;
}
void Podcast::set_extra(const QVariantMap& v) { d->extra_ = v; }
void Podcast::set_extra(const QString& key, const QVariant& value) {
  d->extra_[key] = value;
}

const PodcastEpisodeList& Podcast::episodes() const { return d->episodes_; }
PodcastEpisodeList* Podcast::mutable_episodes() { return &d->episodes_; }
void Podcast::set_episodes(const PodcastEpisodeList& v) { d->episodes_ = v; }
void Podcast::add_episode(const PodcastEpisode& episode) {
  d->episodes_.append(episode);
}

void Podcast::InitFromQuery(const QSqlQuery& query) {
  d->database_id_ = query.value(0).toInt();
  d->url_ = QUrl::fromEncoded(query.value(1).toByteArray());
  d->title_ = query.value(2).toString();
  d->description_ = query.value(3).toString();
  d->copyright_ = query.value(4).toString();
  d->link_ = QUrl::fromEncoded(query.value(5).toByteArray());
  d->image_url_large_ = QUrl::fromEncoded(query.value(6).toByteArray());
  d->image_url_small_ = QUrl::fromEncoded(query.value(7).toByteArray());
  d->author_ = query.value(8).toString();
  d->owner_name_ = query.value(9).toString();
  d->owner_email_ = query.value(10).toString();
  d->last_updated_ = QDateTime::fromTime_t(query.value(11).toUInt());
  d->last_update_error_ = query.value(12).toString();

  QDataStream extra_stream(query.value(13).toByteArray());
  extra_stream >> d->extra_;
}

void Podcast::BindToQuery(QSqlQuery* query) const {
  query->bindValue(":url", d->url_.toEncoded());
  query->bindValue(":title", d->title_);
  query->bindValue(":description", d->description_);
  query->bindValue(":copyright", d->copyright_);
  query->bindValue(":link", d->link_.toEncoded());
  query->bindValue(":image_url_large", d->image_url_large_.toEncoded());
  query->bindValue(":image_url_small", d->image_url_small_.toEncoded());
  query->bindValue(":author", d->author_);
  query->bindValue(":owner_name", d->owner_name_);
  query->bindValue(":owner_email", d->owner_email_);
  query->bindValue(":last_updated", d->last_updated_.toTime_t());
  query->bindValue(":last_update_error", d->last_update_error_);

  QByteArray extra;
  QDataStream extra_stream(&extra, QIODevice::WriteOnly);
  extra_stream << d->extra_;

  query->bindValue(":extra", extra);
}

void Podcast::InitFromGpo(const mygpo::Podcast* podcast) {
  d->url_ = podcast->url();
  d->title_ = podcast->title();
  d->description_ = podcast->description();
  d->link_ = podcast->website();
  d->image_url_large_ = podcast->logoUrl();

  set_extra("gpodder:subscribers", podcast->subscribers());
  set_extra("gpodder:subscribers_last_week", podcast->subscribersLastWeek());
  set_extra("gpodder:page", podcast->mygpoUrl());
}
