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

#include "internet/podcasts/podcastservicemodel.h"

#include "internet/podcasts/podcastservice.h"
#include "playlist/songmimedata.h"

PodcastServiceModel::PodcastServiceModel(QObject* parent)
    : QStandardItemModel(parent) {}

QMimeData* PodcastServiceModel::mimeData(const QModelIndexList& indexes) const {
  SongMimeData* data = new SongMimeData;
  QList<QUrl> urls;

  for (const QModelIndex& index : indexes) {
    switch (index.data(InternetModel::Role_Type).toInt()) {
      case PodcastService::Type_Episode:
        MimeDataForEpisode(index, data, &urls);
        break;

      case PodcastService::Type_Podcast:
        MimeDataForPodcast(index, data, &urls);
        break;
    }
  }

  data->setUrls(urls);
  return data;
}

void PodcastServiceModel::MimeDataForEpisode(const QModelIndex& index,
                                             SongMimeData* data,
                                             QList<QUrl>* urls) const {
  QVariant episode_variant = index.data(PodcastService::Role_Episode);
  if (!episode_variant.isValid()) return;

  PodcastEpisode episode(episode_variant.value<PodcastEpisode>());

  // Get the podcast from the index's parent
  Podcast podcast;
  QVariant podcast_variant = index.parent().data(PodcastService::Role_Podcast);
  if (podcast_variant.isValid()) {
    podcast = podcast_variant.value<Podcast>();
  }

  Song song = episode.ToSong(podcast);

  data->songs << song;
  *urls << song.url();
}

void PodcastServiceModel::MimeDataForPodcast(const QModelIndex& index,
                                             SongMimeData* data,
                                             QList<QUrl>* urls) const {
  // Get the podcast
  Podcast podcast;
  QVariant podcast_variant = index.data(PodcastService::Role_Podcast);
  if (podcast_variant.isValid()) {
    podcast = podcast_variant.value<Podcast>();
  }

  // Add each child episode
  const int children = index.model()->rowCount(index);
  for (int i = 0; i < children; ++i) {
    QVariant episode_variant =
        index.model()->index(i, 0, index).data(PodcastService::Role_Episode);
    if (!episode_variant.isValid()) continue;

    PodcastEpisode episode(episode_variant.value<PodcastEpisode>());
    Song song = episode.ToSong(podcast);

    data->songs << song;
    *urls << song.url();
  }
}
