/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef PODCASTS_PODCAST_H_
#define PODCASTS_PODCAST_H_

#include "podcastepisode.h"

#include <QSharedDataPointer>
#include <QSqlQuery>
#include <QUrl>
#include <QVariantMap>

namespace mygpo {
class Podcast;
}

class Podcast {
 public:
  Podcast();
  Podcast(const Podcast& other);
  ~Podcast();

  static const QStringList kColumns;
  static const QString kColumnSpec;
  static const QString kJoinSpec;
  static const QString kBindSpec;
  static const QString kUpdateSpec;

  void InitFromQuery(const QSqlQuery& query);
  void InitFromGpo(const mygpo::Podcast* podcast);

  void BindToQuery(QSqlQuery* query) const;

  bool is_valid() const { return database_id() != -1; }

  int database_id() const;
  const QUrl& url() const;
  const QString& title() const;
  const QString& description() const;
  const QString& copyright() const;
  const QUrl& link() const;
  const QUrl& image_url_large() const;
  const QUrl& image_url_small() const;
  const QString& author() const;
  const QString& owner_name() const;
  const QString& owner_email() const;
  const QDateTime& last_updated() const;
  const QString& last_update_error() const;
  const QVariantMap& extra() const;
  QVariant extra(const QString& key) const;

  void set_database_id(int v);
  void set_url(const QUrl& v);
  void set_title(const QString& v);
  void set_description(const QString& v);
  void set_copyright(const QString& v);
  void set_link(const QUrl& v);
  void set_image_url_large(const QUrl& v);
  void set_image_url_small(const QUrl& v);
  void set_author(const QString& v);
  void set_owner_name(const QString& v);
  void set_owner_email(const QString& v);
  void set_last_updated(const QDateTime& v);
  void set_last_update_error(const QString& v);
  void set_extra(const QVariantMap& v);
  void set_extra(const QString& key, const QVariant& value);

  // Small images are suitable for 16x16 icons in lists.  Large images are
  // used in detailed information displays.
  const QUrl& ImageUrlLarge() const {
    return image_url_large().isValid() ? image_url_large() : image_url_small();
  }
  const QUrl& ImageUrlSmall() const {
    return image_url_small().isValid() ? image_url_small() : image_url_large();
  }

  // These are stored in a different database table, and aren't loaded or
  // persisted by InitFromQuery or BindToQuery.
  const PodcastEpisodeList& episodes() const;
  PodcastEpisodeList* mutable_episodes();
  void set_episodes(const PodcastEpisodeList& v);
  void add_episode(const PodcastEpisode& episode);

  Podcast& operator=(const Podcast& other);

 private:
  struct Private;
  QSharedDataPointer<Private> d;
};
Q_DECLARE_METATYPE(Podcast)

typedef QList<Podcast> PodcastList;
Q_DECLARE_METATYPE(QList<Podcast>)

#endif  // PODCASTS_PODCAST_H_
