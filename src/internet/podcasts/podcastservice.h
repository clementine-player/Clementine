/* This file is part of Clementine.
   Copyright 2012-2013, David Sansome <me@davidsansome.com>
   Copyright 2013-2014, Krzysztof A. Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Simeon Bird <sbird@andrew.cmu.edu>

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

#ifndef INTERNET_PODCASTS_PODCASTSERVICE_H_
#define INTERNET_PODCASTS_PODCASTSERVICE_H_

#include "podcastdeleter.h"
#include "podcastdownloader.h"
#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"

#include <memory>
#include <QScopedPointer>

class AddPodcastDialog;
class PodcastInfoDialog;
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

  enum Role { Role_Podcast = InternetModel::RoleCount, Role_Episode };

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);
  bool has_initial_load_settings() const { return true; }
  void ShowContextMenu(const QPoint& global_pos);
  void ReloadSettings();
  void InitialLoadSettings();
  // Called by SongLoader when the user adds a Podcast URL directly.  Adds a
  // subscription to the podcast and displays it in the UI.  If the QVariant
  // contains an OPML file then this displays it in the Add Podcast dialog.
  void SubscribeAndShow(const QVariant& podcast_or_opml);

 public slots:
  void AddPodcast();
  void FileCopied(int database_id);

 private slots:
  void UpdateSelectedPodcast();
  void ReloadPodcast(const Podcast& podcast);
  void RemoveSelectedPodcast();
  void DownloadSelectedEpisode();
  void PodcastInfo();
  void DeleteDownloadedData();
  void SetNew();
  void SetListened();
  void ShowConfig();

  void SubscriptionAdded(const Podcast& podcast);
  void SubscriptionRemoved(const Podcast& podcast);
  void EpisodesAdded(const PodcastEpisodeList& episodes);
  void EpisodesUpdated(const PodcastEpisodeList& episodes);

  void DownloadProgressChanged(const PodcastEpisode& episode,
                               PodcastDownload::State state, int percent);

  void CurrentSongChanged(const Song& metadata);

  void CopyToDevice();
  void CopyToDevice(const PodcastEpisodeList& episodes_list);
  void CopyToDevice(const QModelIndexList& episode_indexes,
                    const QModelIndexList& podcast_indexes);
  void CancelDownload();
  void CancelDownload(const QModelIndexList& episode_indexes,
                      const QModelIndexList& podcast_indexes);

 private:
  void EnsureAddPodcastDialogCreated();

  void UpdatePodcastListenedStateAsync(const Song& metadata);
  void PopulatePodcastList(QStandardItem* parent);
  void ClearPodcastList(QStandardItem* parent);
  void UpdatePodcastText(QStandardItem* item, int unlistened_count) const;
  void UpdateEpisodeText(
      QStandardItem* item,
      PodcastDownload::State state = PodcastDownload::NotDownloading,
      int percent = 0);
  void UpdatePodcastText(
      QStandardItem* item,
      PodcastDownload::State state = PodcastDownload::NotDownloading,
      int percent = 0);

  QStandardItem* CreatePodcastItem(const Podcast& podcast);
  QStandardItem* CreatePodcastEpisodeItem(const PodcastEpisode& episode);

  QModelIndex MapToMergedModel(const QModelIndex& index) const;

  void SetListened(const QModelIndexList& episode_indexes,
                   const QModelIndexList& podcast_indexes, bool listened);
  void SetListened(const PodcastEpisodeList& episodes_list, bool listened);

  void LazyLoadRoot();

 private:
  bool use_pretty_covers_;
  bool hide_listened_;
  qint64 show_episodes_;
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
  QAction* info_selected_action_;
  QAction* delete_downloaded_action_;
  QAction* set_new_action_;
  QAction* set_listened_action_;
  QAction* copy_to_device_;
  QAction* cancel_download_;
  QStandardItem* root_;
  std::unique_ptr<OrganiseDialog> organise_dialog_;

  QModelIndexList explicitly_selected_podcasts_;
  QModelIndexList selected_podcasts_;
  QModelIndexList selected_episodes_;

  QMap<int, QStandardItem*> podcasts_by_database_id_;
  QMap<int, QStandardItem*> episodes_by_database_id_;

  std::unique_ptr<AddPodcastDialog> add_podcast_dialog_;
  std::unique_ptr<PodcastInfoDialog> podcast_info_dialog_;
};

#endif  // INTERNET_PODCASTS_PODCASTSERVICE_H_
