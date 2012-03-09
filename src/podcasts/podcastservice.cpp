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
#include "podcastupdater.h"
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


class PodcastSortProxyModel : public QSortFilterProxyModel {
public:
  PodcastSortProxyModel(QObject* parent = NULL);

protected:
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};


PodcastService::PodcastService(Application* app, InternetModel* parent)
  : InternetService(kServiceName, app, parent, parent),
    use_pretty_covers_(true),
    icon_loader_(new StandardItemIconLoader(app->album_cover_loader(), this)),
    backend_(app->podcast_backend()),
    model_(new QStandardItemModel(this)),
    proxy_(new PodcastSortProxyModel(this)),
    context_menu_(NULL),
    root_(NULL)
{
  icon_loader_->SetModel(model_);
  proxy_->setSourceModel(model_);
  proxy_->setDynamicSortFilter(true);
  proxy_->sort(0);

  connect(backend_, SIGNAL(SubscriptionAdded(Podcast)), SLOT(SubscriptionAdded(Podcast)));
  connect(backend_, SIGNAL(SubscriptionRemoved(Podcast)), SLOT(SubscriptionRemoved(Podcast)));
  connect(backend_, SIGNAL(EpisodesAdded(QList<PodcastEpisode>)), SLOT(EpisodesAdded(QList<PodcastEpisode>)));
}

PodcastService::~PodcastService() {
}

PodcastSortProxyModel::PodcastSortProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent) {
}

bool PodcastSortProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
  const int left_type  = left.data(InternetModel::Role_Type).toInt();
  const int right_type = right.data(InternetModel::Role_Type).toInt();

  // The special Add Podcast item comes first
  if (left_type == PodcastService::Type_AddPodcast)
    return true;
  else if (right_type == PodcastService::Type_AddPodcast)
    return false;

  // Otherwise we only compare identical typed items.
  if (left_type != right_type)
    return QSortFilterProxyModel::lessThan(left, right);

  switch (left_type) {
  case PodcastService::Type_Podcast:
    return left.data().toString().localeAwareCompare(right.data().toString()) < 0;

  case PodcastService::Type_Episode: {
    const PodcastEpisode left_episode  = left.data(PodcastService::Type_Episode).value<PodcastEpisode>();
    const PodcastEpisode right_episode = right.data(PodcastService::Type_Episode).value<PodcastEpisode>();

    return left_episode.publication_date() > right_episode.publication_date();
  }

  default:
    return QSortFilterProxyModel::lessThan(left, right);
  }
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

void PodcastService::UpdatePodcastText(QStandardItem* item, int unlistened_count) const {
  const Podcast podcast = item->data(Role_Podcast).value<Podcast>();

  QString title = podcast.title();
  QFont font;

  if (unlistened_count > 0) {
    // Add the number of new episodes after the title.
    title.append(QString(" (%1)").arg(unlistened_count));

    // Set a bold font
    font.setWeight(QFont::DemiBold);
  }

  item->setFont(font);
  item->setText(title);
}

QStandardItem* PodcastService::CreatePodcastItem(const Podcast& podcast) {
  QStandardItem* item = new QStandardItem;

  // Add the episodes in this podcast and gather aggregate stats.
  int unlistened_count = 0;
  foreach (const PodcastEpisode& episode, backend_->GetEpisodes(podcast.database_id())) {
    if (!episode.listened()) {
      unlistened_count ++;
    }

    item->appendRow(CreatePodcastEpisodeItem(episode));
  }

  item->setIcon(default_icon_);
  item->setData(Type_Podcast, InternetModel::Role_Type);
  item->setData(QVariant::fromValue(podcast), Role_Podcast);
  UpdatePodcastText(item, unlistened_count);

  // Load the podcast's image if it has one
  if (podcast.ImageUrlSmall().isValid()) {
    icon_loader_->LoadIcon(podcast.ImageUrlSmall().toString(), QString(), item);
  }

  podcasts_by_database_id_[podcast.database_id()] = item;

  return item;
}

QStandardItem* PodcastService::CreatePodcastEpisodeItem(const PodcastEpisode& episode) {
  QStandardItem* item = new QStandardItem;
  item->setText(episode.title());
  item->setData(Type_Episode, InternetModel::Role_Type);
  item->setData(QVariant::fromValue(episode), Role_Episode);

  if (!episode.listened()) {
    QFont font(item->font());
    font.setBold(true);
    item->setFont(font);
  }

  return item;
}

void PodcastService::ShowContextMenu(const QModelIndex& index,
                                     const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addAction(IconLoader::Load("list-add"), tr("Add podcast..."),
                             this, SLOT(AddPodcast()));
    context_menu_->addAction(IconLoader::Load("view-refresh"), tr("Update all podcasts"),
                             app_->podcast_updater(), SLOT(UpdateAllPodcastsNow()));

    context_menu_->addSeparator();
    update_selected_action_ = context_menu_->addAction(IconLoader::Load("view-refresh"),
                                                       tr("Update this podcast"),
                                                       this, SLOT(UpdateSelectedPodcast()));
    remove_selected_action_ = context_menu_->addAction(IconLoader::Load("list-remove"),
                                                       tr("Unsubscribe"),
                                                       this, SLOT(RemoveSelectedPodcast()));
  }

  current_index_ = index;

  switch (index.data(InternetModel::Role_Type).toInt()) {
  case Type_Podcast:
    current_podcast_index_ = index;
    break;

  case Type_Episode:
    current_podcast_index_ = index.parent();
    break;

  default:
    current_podcast_index_ = QModelIndex();
    break;
  }

  update_selected_action_->setVisible(current_podcast_index_.isValid());
  remove_selected_action_->setVisible(current_podcast_index_.isValid());
  context_menu_->popup(global_pos);
}

void PodcastService::UpdateSelectedPodcast() {
  if (!current_podcast_index_.isValid())
    return;

  app_->podcast_updater()->UpdatePodcastNow(
        current_podcast_index_.data(Role_Podcast).value<Podcast>());
}

void PodcastService::RemoveSelectedPodcast() {
  if (!current_podcast_index_.isValid())
    return;

  backend_->Unsubscribe(current_podcast_index_.data(Role_Podcast).value<Podcast>());
}

void PodcastService::ReloadSettings() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);

  use_pretty_covers_ = s.value("pretty_covers", true).toBool();
  // TODO: reload the podcast icons that are already loaded?
}

QModelIndex PodcastService::GetCurrentIndex() {
  return current_index_;
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
  QStandardItem* item = podcasts_by_database_id_.take(podcast.database_id());
  if (item) {
    model_->removeRow(item->row());
  }
}

void PodcastService::EpisodesAdded(const QList<PodcastEpisode>& episodes) {
  QSet<int> seen_podcast_ids;

  foreach (const PodcastEpisode& episode, episodes) {
    const int database_id = episode.podcast_database_id();
    QStandardItem* parent = podcasts_by_database_id_[database_id];
    if (!parent)
      continue;

    parent->appendRow(CreatePodcastEpisodeItem(episode));

    if (!seen_podcast_ids.contains(database_id)) {
      // Update the unlistened count text once for each podcast
      int unlistened_count = 0;
      foreach (const PodcastEpisode& episode, backend_->GetEpisodes(database_id)) {
        if (!episode.listened()) {
          unlistened_count ++;
        }
      }

      UpdatePodcastText(parent, unlistened_count);
      seen_podcast_ids.insert(database_id);
    }
  }
}
