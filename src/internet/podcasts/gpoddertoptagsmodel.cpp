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

#include "gpoddertoptagsmodel.h"

#include <QMessageBox>
#include <QPersistentModelIndex>

#include "gpodderclient.h"
#include "gpoddertoptagspage.h"
#include "podcast.h"

GPodderTopTagsModel::GPodderTopTagsModel(GPodderClient* client,
                                         Application* app, QObject* parent)
    : PodcastDiscoveryModel(app, parent), client_(client) {}

bool GPodderTopTagsModel::hasChildren(const QModelIndex& parent) const {
  if (parent.isValid() && parent.data(Role_Type).toInt() == Type_Folder) {
    return true;
  }

  return PodcastDiscoveryModel::hasChildren(parent);
}

bool GPodderTopTagsModel::canFetchMore(const QModelIndex& parent) const {
  if (parent.isValid() && parent.data(Role_Type).toInt() == Type_Folder &&
      !parent.data(Role_HasLazyLoaded).toBool()) {
    return true;
  }

  return PodcastDiscoveryModel::canFetchMore(parent);
}

void GPodderTopTagsModel::fetchMore(const QModelIndex& parent) {
  if (!parent.isValid() || parent.data(Role_Type).toInt() != Type_Folder ||
      parent.data(Role_HasLazyLoaded).toBool()) {
    return;
  }
  setData(parent, true, Role_HasLazyLoaded);

  // Create a little Loading... item.
  itemFromIndex(parent)->appendRow(CreateLoadingIndicator());

  // The tag's row can move or go while the request is out.
  const QPersistentModelIndex tag_index(parent);
  client_->PodcastsOfTag(
      parent.data().toString(), GPodderTopTagsPage::kMaxTagCount,
      [this, tag_index](const QList<Podcast>& podcasts) {
        PodcastsOfTagFinished(tag_index, podcasts);
      },
      [this, tag_index](const QString&) { PodcastsOfTagFailed(tag_index); });
}

void GPodderTopTagsModel::PodcastsOfTagFinished(
    const QModelIndex& parent, const QList<Podcast>& podcasts) {
  QStandardItem* parent_item = itemFromIndex(parent);
  if (!parent_item) return;

  // Remove the Loading... item.
  while (parent_item->hasChildren()) {
    parent_item->removeRow(0);
  }

  for (const Podcast& podcast : podcasts) {
    parent_item->appendRow(CreatePodcastItem(podcast));
  }
}

void GPodderTopTagsModel::PodcastsOfTagFailed(const QModelIndex& parent) {
  QStandardItem* parent_item = itemFromIndex(parent);
  if (!parent_item) return;

  // Remove the Loading... item.
  while (parent_item->hasChildren()) {
    parent_item->removeRow(0);
  }

  if (QMessageBox::warning(
          nullptr, tr("Failed to fetch podcasts"),
          tr("There was a problem communicating with gpodder.net"),
          QMessageBox::Retry | QMessageBox::Close,
          QMessageBox::Retry) != QMessageBox::Retry) {
    return;
  }

  // Try fetching the list again.
  setData(parent, false, Role_HasLazyLoaded);
  fetchMore(parent);
}
