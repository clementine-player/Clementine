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
#include "covers/albumcoverloader.h"
#include "ui/iconloader.h"

#include <QIcon>
#include <QSet>

PodcastDiscoveryModel::PodcastDiscoveryModel(Application* app, QObject* parent)
  : QStandardItemModel(parent),
    app_(app),
    is_tree_(false),
    default_icon_(":providers/podcast32.png")
{
  cover_options_.desired_height_ = 32;

  connect(app_->album_cover_loader(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(ImageLoaded(quint64,QImage)));
  connect(this, SIGNAL(modelAboutToBeReset()), SLOT(CancelPendingImages()));
}

QStandardItem* PodcastDiscoveryModel::CreatePodcastItem(const Podcast& podcast) {
  QStandardItem* item = new QStandardItem;
  item->setIcon(default_icon_);
  item->setText(podcast.title());
  item->setData(QVariant::fromValue(podcast), Role_Podcast);
  item->setData(Type_Podcast, Role_Type);

  if (podcast.image_url().isValid()) {
    // Start loading an image for this item.
    quint64 id = app_->album_cover_loader()->LoadImageAsync(
          cover_options_, podcast.image_url().toString(), QString());
    pending_covers_[id] = item;
  }

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

void PodcastDiscoveryModel::ImageLoaded(quint64 id, const QImage& image) {
  QStandardItem* item = pending_covers_.take(id);
  if (!item)
    return;

  item->setIcon(QIcon(QPixmap::fromImage(image)));
}

void PodcastDiscoveryModel::CancelPendingImages() {
  app_->album_cover_loader()->CancelTasks(QSet<quint64>::fromList(pending_covers_.keys()));
  pending_covers_.clear();
}
