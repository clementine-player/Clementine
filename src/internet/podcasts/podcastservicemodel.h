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

#ifndef INTERNET_PODCASTS_PODCASTSERVICEMODEL_H_
#define INTERNET_PODCASTS_PODCASTSERVICEMODEL_H_

#include <QStandardItemModel>

class SongMimeData;

class PodcastServiceModel : public QStandardItemModel {
  Q_OBJECT

 public:
  explicit PodcastServiceModel(QObject* parent = nullptr);

  QMimeData* mimeData(const QModelIndexList& indexes) const;

 private:
  void MimeDataForPodcast(const QModelIndex& index, SongMimeData* data,
                          QList<QUrl>* urls) const;
  void MimeDataForEpisode(const QModelIndex& index, SongMimeData* data,
                          QList<QUrl>* urls) const;
};

#endif  // INTERNET_PODCASTS_PODCASTSERVICEMODEL_H_
