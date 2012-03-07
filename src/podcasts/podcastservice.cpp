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
#include "podcastbackend.h"
#include "podcastservice.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "internet/internetmodel.h"
#include "library/libraryview.h"
#include "ui/iconloader.h"
#include "ui/standarditemiconloader.h"

#include <QMenu>
#include <QSortFilterProxyModel>

const char* PodcastService::kServiceName = "Podcasts";
const char* PodcastService::kSettingsGroup = "Podcasts";

PodcastService::PodcastService(Application* app, InternetModel* parent)
  : InternetService(kServiceName, app, parent, parent),
    use_pretty_covers_(true),
    icon_loader_(new StandardItemIconLoader(app->album_cover_loader(), this)),
    backend_(app->podcast_backend()),
    model_(new QStandardItemModel(this)),
    proxy_(new QSortFilterProxyModel(this)),
    context_menu_(NULL),
    root_(NULL)
{
  icon_loader_->SetModel(model_);
  proxy_->setSourceModel(model_);
  proxy_->setDynamicSortFilter(true);
  proxy_->sort(0);

  connect(backend_, SIGNAL(SubscriptionAdded(Podcast)), SLOT(SubscriptionAdded(Podcast)));
  connect(backend_, SIGNAL(SubscriptionRemoved(Podcast)), SLOT(SubscriptionRemoved(Podcast)));
}

PodcastService::~PodcastService() {
}

QStandardItem* PodcastService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/podcast16.png"), tr("Podcasts"));
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void PodcastService::LazyPopulate(QStandardItem* parent) {
  switch (parent->data(InternetModel::Role_Type).toInt()) {
  case InternetModel::Type_Service:
    PopulatePodcastList(model_->invisibleRootItem());
    model()->merged_model()->AddSubModel(parent->index(), proxy_);
    break;
  }
}

void PodcastService::PopulatePodcastList(QStandardItem* parent) {
  if (default_icon_.isNull()) {
    default_icon_ = QIcon(":providers/podcast16.png");
  }

  foreach (const Podcast& podcast, backend_->GetAllSubscriptions()) {
    parent->appendRow(CreatePodcastItem(podcast));
  }
}

QStandardItem* PodcastService::CreatePodcastItem(const Podcast& podcast) {
  const int unlistened_count = podcast.extra("db:unlistened_count").toInt();
  QString title = podcast.title();

  QStandardItem* item = new QStandardItem;

  if (unlistened_count > 0) {
    // Add the number of new episodes after the title.
    title.append(QString(" (%1)").arg(unlistened_count));

    // Set a bold font
    QFont font(item->font());
    font.setBold(true);
    item->setFont(font);
  }

  item->setText(podcast.title());
  item->setIcon(default_icon_);
  item->setData(QVariant::fromValue(podcast), Role_Podcast);

  // Load the podcast's image if it has one
  if (podcast.image_url().isValid()) {
    icon_loader_->LoadIcon(podcast.image_url().toString(), QString(), item);
  }

  return item;
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

void PodcastService::ReloadSettings() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);

  use_pretty_covers_ = s.value("pretty_covers", true).toBool();
  // TODO: reload the podcast icons that are already loaded?
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

void PodcastService::SubscriptionAdded(const Podcast& podcast) {
  // If the user hasn't expanded the root node yet we don't need to do anything
  if (root_->data(InternetModel::Role_CanLazyLoad).toBool()) {
    return;
  }

  model_->appendRow(CreatePodcastItem(podcast));
}

void PodcastService::SubscriptionRemoved(const Podcast& podcast) {
  // Find the item in the model that matches this podcast.
  for (int i=0 ; i<model_->rowCount() ; ++i) {
    Podcast item_podcast(model_->item(i)->data(Role_Podcast).value<Podcast>());
    if (podcast.database_id() == item_podcast.database_id()) {
      model_->removeRow(i);
      return;
    }
  }
}
