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

#ifndef INTERNET_PODCASTS_GPODDERTOPTAGSMODEL_H_
#define INTERNET_PODCASTS_GPODDERTOPTAGSMODEL_H_

#include "podcast.h"
#include "podcastdiscoverymodel.h"

class GPodderClient;

class GPodderTopTagsModel : public PodcastDiscoveryModel {
  Q_OBJECT

 public:
  GPodderTopTagsModel(GPodderClient* client, Application* app,
                      QObject* parent = nullptr);

  enum Role {
    Role_HasLazyLoaded = PodcastDiscoveryModel::RoleCount,
    RoleCount
  };

  bool hasChildren(const QModelIndex& parent) const;
  bool canFetchMore(const QModelIndex& parent) const;
  void fetchMore(const QModelIndex& parent);

 private slots:
  void PodcastsOfTagFinished(const QModelIndex& parent,
                             const QList<Podcast>& podcasts);
  void PodcastsOfTagFailed(const QModelIndex& parent);

 private:
  GPodderClient* client_;
};

#endif  // INTERNET_PODCASTS_GPODDERTOPTAGSMODEL_H_
