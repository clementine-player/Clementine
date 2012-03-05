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

#include "addpodcastdialog.h"
#include "podcastservice.h"
#include "internet/internetmodel.h"
#include "ui/iconloader.h"

#include <QMenu>

const char* PodcastService::kServiceName = "Podcasts";
const char* PodcastService::kSettingsGroup = "Podcasts";

PodcastService::PodcastService(Application* app, InternetModel* parent)
  : InternetService(kServiceName, app, parent, parent),
    context_menu_(NULL),
    root_(NULL)
{
}

PodcastService::~PodcastService() {
}

QStandardItem* PodcastService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/podcast16.png"), tr("Podcasts"));
  return root_;
}

void PodcastService::LazyPopulate(QStandardItem* parent) {
}

void PodcastService::ShowContextMenu(const QModelIndex& index,
                                     const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addAction(IconLoader::Load("list-add"), tr("Add podcast..."),
                             this, SLOT(AddPodcast()));
  }

  context_menu_->popup(global_pos);
}

QModelIndex PodcastService::GetCurrentIndex() {
  return QModelIndex();
}

void PodcastService::AddPodcast() {
  if (!add_podcast_dialog_) {
    add_podcast_dialog_.reset(new AddPodcastDialog(app_));
  }

  add_podcast_dialog_->show();
}
