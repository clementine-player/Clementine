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

#ifndef PODCASTS_GPODDERTOPTAGSMODEL_H_
#define PODCASTS_GPODDERTOPTAGSMODEL_H_

#include "podcastdiscoverymodel.h"

namespace mygpo {
class ApiRequest;
class PodcastList;
}

class GPodderTopTagsModel : public PodcastDiscoveryModel {
  Q_OBJECT

 public:
  GPodderTopTagsModel(mygpo::ApiRequest* api, Application* app,
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
                             mygpo::PodcastList* list);
  void PodcastsOfTagFailed(const QModelIndex& parent, mygpo::PodcastList* list);

 private:
  mygpo::ApiRequest* api_;
};

#endif  // PODCASTS_GPODDERTOPTAGSMODEL_H_
