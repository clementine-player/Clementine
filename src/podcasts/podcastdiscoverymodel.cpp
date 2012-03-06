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

#include "podcast.h"
#include "podcastdiscoverymodel.h"
#include "core/application.h"
#include "ui/iconloader.h"
#include "ui/standarditemiconloader.h"

#include <QIcon>
#include <QSet>

PodcastDiscoveryModel::PodcastDiscoveryModel(Application* app, QObject* parent)
  : QStandardItemModel(parent),
    app_(app),
    icon_loader_(new StandardItemIconLoader(app->album_cover_loader(), this)),
    is_tree_(false),
    default_icon_(":providers/podcast16.png")
{
  icon_loader_->SetModel(this);
}

QVariant PodcastDiscoveryModel::data(const QModelIndex& index, int role) const {
  if (index.isValid() &&
      role == Qt::DecorationRole &&
      QStandardItemModel::data(index, Role_Type).toInt() == Type_Podcast &&
      QStandardItemModel::data(index, Role_StartedLoadingImage).toBool() == false) {
    const_cast<PodcastDiscoveryModel*>(this)->LazyLoadImage(index);
  }

  return QStandardItemModel::data(index, role);
}

QStandardItem* PodcastDiscoveryModel::CreatePodcastItem(const Podcast& podcast) {
  QStandardItem* item = new QStandardItem;
  item->setIcon(default_icon_);
  item->setText(podcast.title());
  item->setData(QVariant::fromValue(podcast), Role_Podcast);
  item->setData(Type_Podcast, Role_Type);
  return item;
}

QStandardItem* PodcastDiscoveryModel::CreateFolder(const QString& name) {
  if (folder_icon_.isNull()) {
    folder_icon_ = IconLoader::Load("folder");
  }

  QStandardItem* item = new QStandardItem;
  item->setIcon(folder_icon_);
  item->setText(name);
  item->setData(Type_Folder, Role_Type);
  return item;
}

void PodcastDiscoveryModel::LazyLoadImage(const QModelIndex& index) {
  QStandardItem* item = itemFromIndex(index);
  item->setData(true, Role_StartedLoadingImage);

  Podcast podcast = index.data(Role_Podcast).value<Podcast>();

  if (podcast.image_url().isValid()) {
    icon_loader_->LoadIcon(podcast.image_url().toString(), QString(), item);
  }
}

QStandardItem* PodcastDiscoveryModel::CreateLoadingIndicator() {
  QStandardItem* item = new QStandardItem;
  item->setText(tr("Loading..."));
  item->setData(Type_LoadingIndicator, Role_Type);
  return item;
}
