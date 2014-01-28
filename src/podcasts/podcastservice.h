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

#ifndef PODCASTSERVICE_H
#define PODCASTSERVICE_H

#include "podcastdownloader.h"
#include "internet/internetmodel.h"
#include "internet/internetservice.h"

#include <memory>
#include <QScopedPointer>

class AddPodcastDialog;
class OrganiseDialog;
class Podcast;
class PodcastBackend;
class PodcastEpisode;
class StandardItemIconLoader;

class QSortFilterProxyModel;

class PodcastService : public InternetService {
  Q_OBJECT

public:
  PodcastService(Application* app, InternetModel* parent);
  ~PodcastService();

  static const char* kServiceName;
  static const char* kSettingsGroup;

  enum Type {
    Type_AddPodcast = InternetModel::TypeCount,
    Type_Podcast,
    Type_Episode
  };

  enum Role {
    Role_Podcast = InternetModel::RoleCount,
    Role_Episode
  };
  
  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);

  void ShowContextMenu(const QPoint& global_pos);
  void ReloadSettings();

  // Called by SongLoader when the user adds a Podcast URL directly.  Adds a
  // subscription to the podcast and displays it in the UI.  If the QVariant
  // contains an OPML file then this displays it in the Add Podcast dialog.
  void SubscribeAndShow(const QVariant& podcast_or_opml);
  
public slots:
  void AddPodcast();

private slots:
  void UpdateSelectedPodcast();
  void RemoveSelectedPodcast();
  void DownloadSelectedEpisode();
  void DeleteDownloadedData();
  void SetNew();
  void SetListened();
  void ShowConfig();

  void SubscriptionAdded(const Podcast& podcast);
  void SubscriptionRemoved(const Podcast& podcast);
  void EpisodesAdded(const PodcastEpisodeList& episodes);
  void EpisodesUpdated(const PodcastEpisodeList& episodes);

  void DownloadProgressChanged(const PodcastEpisode& episode,
                               PodcastDownloader::State state,
                               int percent);

  void CurrentSongChanged(const Song& metadata);

  void CopyToDevice();
  void CopyToDevice(const PodcastEpisodeList& episodes_list);
  void CopyToDevice(const QModelIndexList& episode_indexes,
			const QModelIndexList& podcast_indexes);

private:
  void EnsureAddPodcastDialogCreated();

  void PopulatePodcastList(QStandardItem* parent);
  void UpdatePodcastText(QStandardItem* item, int unlistened_count) const;
  void UpdateEpisodeText(QStandardItem* item,
                         PodcastDownloader::State state = PodcastDownloader::NotDownloading,
                         int percent = 0);

  QStandardItem* CreatePodcastItem(const Podcast& podcast);
  QStandardItem* CreatePodcastEpisodeItem(const PodcastEpisode& episode);

  QModelIndex MapToMergedModel(const QModelIndex& index) const;

  void SetListened(const QModelIndexList& episode_indexes,
                   const QModelIndexList& podcast_indexes,
                   bool listened);
  void SetListened(const PodcastEpisodeList& episodes_list,
		   bool listened);

  void LazyLoadRoot();

private:
  bool use_pretty_covers_;
  StandardItemIconLoader* icon_loader_;

  // The podcast icon
  QIcon default_icon_;

  // Episodes get different icons depending on their state
  QIcon queued_icon_;
  QIcon downloading_icon_;
  QIcon downloaded_icon_;

  PodcastBackend* backend_;
  QStandardItemModel* model_;
  QSortFilterProxyModel* proxy_;

  QMenu* context_menu_;
  QAction* update_selected_action_;
  QAction* remove_selected_action_;
  QAction* download_selected_action_;
  QAction* delete_downloaded_action_;
  QAction* set_new_action_;
  QAction* set_listened_action_;
  QAction* copy_to_device_;
  QStandardItem* root_;
  std::unique_ptr<OrganiseDialog> organise_dialog_;

  QModelIndexList explicitly_selected_podcasts_;
  QModelIndexList selected_podcasts_;
  QModelIndexList selected_episodes_;

  QMap<int, QStandardItem*> podcasts_by_database_id_;
  QMap<int, QStandardItem*> episodes_by_database_id_;

  QScopedPointer<AddPodcastDialog> add_podcast_dialog_;
};

#endif // PODCASTSERVICE_H
