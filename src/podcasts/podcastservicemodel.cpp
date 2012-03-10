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

#include "podcastservice.h"
#include "podcastservicemodel.h"
#include "playlist/songmimedata.h"

PodcastServiceModel::PodcastServiceModel(QObject* parent)
  : QStandardItemModel(parent)
{
}

QMimeData* PodcastServiceModel::mimeData(const QModelIndexList& indexes) const {
  SongMimeData* data = new SongMimeData;
  QList<QUrl> urls;

  foreach (const QModelIndex& index, indexes) {
    QVariant episode_variant = index.data(PodcastService::Role_Episode);
    if (!episode_variant.isValid())
      continue;

    PodcastEpisode episode(episode_variant.value<PodcastEpisode>());

    // Get the podcast from the index's parent
    Podcast podcast;
    QVariant podcast_variant = index.parent().data(PodcastService::Role_Podcast);
    if (podcast_variant.isValid()) {
      podcast = podcast_variant.value<Podcast>();
    }

    Song song = episode.ToSong(podcast);

    data->songs << song;
    urls << song.url();
  }

  data->setUrls(urls);
  return data;
}
