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

#include "gpoddertoptagsmodel.h"
#include "gpoddertoptagspage.h"
#include "podcast.h"
#include "core/closure.h"

#include <ApiRequest.h>

GPodderTopTagsModel::GPodderTopTagsModel(mygpo::ApiRequest* api, Application* app,
                                         QObject* parent)
  : PodcastDiscoveryModel(app, parent),
    api_(api)
{
  set_is_tree(true);
}

bool GPodderTopTagsModel::hasChildren(const QModelIndex& parent) const {
  if (parent.isValid() &&
      parent.data(Role_Type).toInt() == Type_Folder) {
    return true;
  }

  return PodcastDiscoveryModel::hasChildren(parent);
}

bool GPodderTopTagsModel::canFetchMore(const QModelIndex& parent) const {
  if (parent.isValid() &&
      parent.data(Role_Type).toInt() == Type_Folder &&
      parent.data(Role_HasLazyLoaded).toBool() == false) {
    return true;
  }

  return PodcastDiscoveryModel::canFetchMore(parent);
}

void GPodderTopTagsModel::fetchMore(const QModelIndex& parent) {
  if (!parent.isValid() ||
      parent.data(Role_Type).toInt() != Type_Folder ||
      parent.data(Role_HasLazyLoaded).toBool()) {
    return;
  }
  setData(parent, true, Role_HasLazyLoaded);

  qLog(Debug) << "Fetching podcasts for" << parent.data().toString();
  mygpo::PodcastList* list =
      api_->podcastsOfTag(GPodderTopTagsPage::kMaxTagCount, parent.data().toString());

  NewClosure(list, SIGNAL(finished()),
             this, SLOT(PodcastsOfTagFinished(QModelIndex,mygpo::PodcastList*)),
             parent, list);
}

void GPodderTopTagsModel::PodcastsOfTagFinished(const QModelIndex& parent,
                                                mygpo::PodcastList* list) {
  list->deleteLater();
  qLog(Debug) << "Tag list finished";

  QStandardItem* parent_item = itemFromIndex(parent);
  if (!parent_item)
    return;

  foreach (mygpo::PodcastPtr gpo_podcast, list->list()) {
    Podcast podcast;
    podcast.InitFromGpo(gpo_podcast.data());

    parent_item->appendRow(CreatePodcastItem(podcast));
  }
}
