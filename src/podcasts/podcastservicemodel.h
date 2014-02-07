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

#ifndef PODCASTSERVICEMODEL_H
#define PODCASTSERVICEMODEL_H

#include <QStandardItemModel>

class SongMimeData;

class PodcastServiceModel : public QStandardItemModel {
  Q_OBJECT

 public:
  PodcastServiceModel(QObject* parent = 0);

  QMimeData* mimeData(const QModelIndexList& indexes) const;

 private:
  void MimeDataForPodcast(const QModelIndex& index, SongMimeData* data,
                          QList<QUrl>* urls) const;
  void MimeDataForEpisode(const QModelIndex& index, SongMimeData* data,
                          QList<QUrl>* urls) const;
};

#endif  // PODCASTSERVICEMODEL_H
