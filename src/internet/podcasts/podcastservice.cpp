/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2012-2013, David Sansome <me@davidsansome.com>
   Copyright 2013-2014, Krzysztof A. Sobiecki <sobkas@gmail.com>
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

#include "internet/podcasts/podcastservice.h"

#include <QMap>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QtConcurrentRun>

#include "addpodcastdialog.h"
#include "podcastinfodialog.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "devices/devicemanager.h"
#include "devices/devicestatefiltermodel.h"
#include "devices/deviceview.h"
#include "internet/core/internetmodel.h"
#include "library/libraryview.h"
#include "opmlcontainer.h"
#include "podcastbackend.h"
#include "podcastdeleter.h"
#include "podcastdownloader.h"
#include "internet/podcasts/podcastservicemodel.h"
#include "podcastupdater.h"
#include "ui/iconloader.h"
#include "ui/organisedialog.h"
#include "ui/organiseerrordialog.h"
#include "ui/standarditemiconloader.h"

const char* PodcastService::kServiceName = "Podcasts";
const char* PodcastService::kSettingsGroup = "Podcasts";

class PodcastSortProxyModel : public QSortFilterProxyModel {
 public:
  explicit PodcastSortProxyModel(QObject* parent = nullptr);

 protected:
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};

PodcastService::PodcastService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      use_pretty_covers_(true),
      hide_listened_(false),
      show_episodes_(0),
      icon_loader_(new StandardItemIconLoader(app->album_cover_loader(), this)),
      backend_(app->podcast_backend()),
      model_(new PodcastServiceModel(this)),
      proxy_(new PodcastSortProxyModel(this)),
      context_menu_(nullptr),
      root_(nullptr),
      organise_dialog_(new OrganiseDialog(app_->task_manager())) {
  icon_loader_->SetModel(model_);
  proxy_->setSourceModel(model_);
  proxy_->setDynamicSortFilter(true);
  proxy_->sort(0);

  connect(backend_, SIGNAL(SubscriptionAdded(Podcast)),
          SLOT(SubscriptionAdded(Podcast)));
  connect(backend_, SIGNAL(SubscriptionRemoved(Podcast)),
          SLOT(SubscriptionRemoved(Podcast)));
  connect(backend_, SIGNAL(EpisodesAdded(PodcastEpisodeList)),
          SLOT(EpisodesAdded(PodcastEpisodeList)));
  connect(backend_, SIGNAL(EpisodesUpdated(PodcastEpisodeList)),
          SLOT(EpisodesUpdated(PodcastEpisodeList)));

  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(CurrentSongChanged(Song)));
  connect(organise_dialog_.get(), SIGNAL(FileCopied(int)), this,
          SLOT(FileCopied(int)));
}

PodcastService::~PodcastService() {}

PodcastSortProxyModel::PodcastSortProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent) {}

bool PodcastSortProxyModel::lessThan(const QModelIndex& left,
                                     const QModelIndex& right) const {
  const int left_type = left.data(InternetModel::Role_Type).toInt();
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
      return left.data().toString().localeAwareCompare(
                 right.data().toString()) < 0;

    case PodcastService::Type_Episode: {
      const PodcastEpisode left_episode =
          left.data(PodcastService::Role_Episode).value<PodcastEpisode>();
      const PodcastEpisode right_episode =
          right.data(PodcastService::Role_Episode).value<PodcastEpisode>();

      return left_episode.publication_date() > right_episode.publication_date();
    }

    default:
      return QSortFilterProxyModel::lessThan(left, right);
  }
}

QStandardItem* PodcastService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("podcast", IconLoader::Provider),
                            tr("Podcasts"));
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void PodcastService::CopyToDevice() {
  if (selected_episodes_.isEmpty() && explicitly_selected_podcasts_.isEmpty()) {
    CopyToDevice(backend_->GetNewDownloadedEpisodes());
  } else {
    CopyToDevice(selected_episodes_, explicitly_selected_podcasts_);
  }
}

void PodcastService::CopyToDevice(const PodcastEpisodeList& episodes_list) {
  SongList songs;
  Podcast podcast;
  for (const PodcastEpisode& episode : episodes_list) {
    podcast = backend_->GetSubscriptionById(episode.podcast_database_id());
    songs.append(episode.ToSong(podcast));
  }

  organise_dialog_->SetDestinationModel(
      app_->device_manager()->connected_devices_model(), true);
  organise_dialog_->SetCopy(true);
  if (organise_dialog_->SetSongs(songs)) organise_dialog_->show();
}

void PodcastService::CopyToDevice(const QModelIndexList& episode_indexes,
                                  const QModelIndexList& podcast_indexes) {
  PodcastEpisode episode_tmp;
  SongList songs;
  PodcastEpisodeList episodes;
  Podcast podcast;
  for (const QModelIndex& index : episode_indexes) {
    episode_tmp = index.data(Role_Episode).value<PodcastEpisode>();
    if (episode_tmp.downloaded()) episodes << episode_tmp;
  }

  for (const QModelIndex& podcast : podcast_indexes) {
    for (int i = 0; i < podcast.model()->rowCount(podcast); ++i) {
      const QModelIndex& index = podcast.child(i, 0);
      episode_tmp = index.data(Role_Episode).value<PodcastEpisode>();
      if (episode_tmp.downloaded() && !episode_tmp.listened())
        episodes << episode_tmp;
    }
  }
  for (const PodcastEpisode& episode : episodes) {
    podcast = backend_->GetSubscriptionById(episode.podcast_database_id());
    songs.append(episode.ToSong(podcast));
  }

  organise_dialog_->SetDestinationModel(
      app_->device_manager()->connected_devices_model(), true);
  organise_dialog_->SetCopy(true);
  if (organise_dialog_->SetSongs(songs)) organise_dialog_->show();
}

void PodcastService::CancelDownload() {
  CancelDownload(selected_episodes_, explicitly_selected_podcasts_);
}

void PodcastService::CancelDownload(const QModelIndexList& episode_indexes,
                                    const QModelIndexList& podcast_indexes) {
  PodcastEpisode episode_tmp;
  SongList songs;
  PodcastEpisodeList episodes;
  Podcast podcast;
  for (const QModelIndex& index : episode_indexes) {
    episode_tmp = index.data(Role_Episode).value<PodcastEpisode>();
    episodes << episode_tmp;
  }

  for (const QModelIndex& podcast : podcast_indexes) {
    for (int i = 0; i < podcast.model()->rowCount(podcast); ++i) {
      const QModelIndex& index = podcast.child(i, 0);
      episode_tmp = index.data(Role_Episode).value<PodcastEpisode>();
      episodes << episode_tmp;
    }
  }
  episodes = app_->podcast_downloader()->EpisodesDownloading(episodes);
  app_->podcast_downloader()->cancelDownload(episodes);
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
  // Do this here since the downloader won't be created yet in the ctor.
  connect(app_->podcast_downloader(),
          SIGNAL(ProgressChanged(PodcastEpisode, PodcastDownload::State, int)),
          SLOT(DownloadProgressChanged(PodcastEpisode, PodcastDownload::State, int)));

  if (default_icon_.isNull()) {
    default_icon_ = IconLoader::Load("podcast", IconLoader::Provider);
  }

  for (const Podcast& podcast : backend_->GetAllSubscriptions()) {
    parent->appendRow(CreatePodcastItem(podcast));
  }
}

void PodcastService::ClearPodcastList(QStandardItem* parent) {
  parent->removeRows(0, parent->rowCount());
}

void PodcastService::UpdatePodcastText(QStandardItem* item,
                                       int unlistened_count) const {
  const Podcast podcast = item->data(Role_Podcast).value<Podcast>();

  QString title = podcast.title().simplified();
  QFont font;

  if (unlistened_count > 0) {
    // Add the number of new episodes after the title.
    title.append(QString(" (%1)").arg(unlistened_count));

    // Set a bold font
    font.setBold(true);
  }

  item->setFont(font);
  item->setText(title);
}

void PodcastService::UpdateEpisodeText(QStandardItem* item,
                                       PodcastDownload::State state,
                                       int percent) {
  const PodcastEpisode episode =
      item->data(Role_Episode).value<PodcastEpisode>();

  QString title = episode.title().simplified();
  QString tooltip;
  QFont font;
  QIcon icon;

  // Unlistened episodes are bold
  if (!episode.listened()) {
    font.setBold(true);
  }

  // Downloaded episodes get an icon
  if (episode.downloaded()) {
    if (downloaded_icon_.isNull()) {
      downloaded_icon_ = IconLoader::Load("document-save", IconLoader::Base);
    }
    icon = downloaded_icon_;
  }

  // Queued or downloading episodes get icons, tooltips, and maybe a title.
  switch (state) {
    case PodcastDownload::Queued:
      if (queued_icon_.isNull()) {
        queued_icon_ = IconLoader::Load("user-away", IconLoader::Base);
      }
      icon = queued_icon_;
      tooltip = tr("Download queued");
      break;

    case PodcastDownload::Downloading:
      if (downloading_icon_.isNull()) {
        downloading_icon_ = IconLoader::Load("go-down", IconLoader::Base);
      }
      icon = downloading_icon_;
      tooltip = tr("Downloading (%1%)...").arg(percent);
      title =
          QString("[ %1% ] %2").arg(QString::number(percent), episode.title());
      break;

    case PodcastDownload::Finished:
    case PodcastDownload::NotDownloading:
      break;
  }

  item->setFont(font);
  item->setText(title);
  item->setIcon(icon);
}

void PodcastService::UpdatePodcastText(QStandardItem* item,
                                       PodcastDownload::State state,
                                       int percent) {
  const Podcast podcast = item->data(Role_Podcast).value<Podcast>();

  QString tooltip;
  QIcon icon;

  // Queued or downloading podcasts get icons, tooltips, and maybe a title.
  switch (state) {
    case PodcastDownload::Queued:
      if (queued_icon_.isNull()) {
        queued_icon_ = IconLoader::Load("user-away", IconLoader::Base);
      }
      icon = queued_icon_;
      item->setIcon(icon);
      tooltip = tr("Download queued");
      break;

    case PodcastDownload::Downloading:
      if (downloading_icon_.isNull()) {
        downloading_icon_ = IconLoader::Load("go-down", IconLoader::Base);
      }
      icon = downloading_icon_;
      item->setIcon(icon);
      tooltip = tr("Downloading (%1%)...").arg(percent);
      break;

    case PodcastDownload::Finished:
    case PodcastDownload::NotDownloading:
      if (podcast.ImageUrlSmall().isValid()) {
        icon_loader_->LoadIcon(podcast.ImageUrlSmall().toString(), QString(), item);
      } else {
        item->setIcon(default_icon_);
      }
      break;
  }
}

QStandardItem* PodcastService::CreatePodcastItem(const Podcast& podcast) {
  QStandardItem* item = new QStandardItem;

  // Add the episodes in this podcast and gather aggregate stats.
  int unlistened_count = 0;
  qint64 number = 0;
  for (const PodcastEpisode& episode :
       backend_->GetEpisodes(podcast.database_id())) {
    if (!episode.listened()) {
      unlistened_count++;
    }

    if (episode.listened() && hide_listened_) {
      continue;
    } else {
      item->appendRow(CreatePodcastEpisodeItem(episode));
      ++number;
    }

    if ((number >= show_episodes_) && (show_episodes_ != 0)) {
      break;
    }
  }

  item->setIcon(default_icon_);
  item->setData(Type_Podcast, InternetModel::Role_Type);
  item->setData(QVariant::fromValue(podcast), Role_Podcast);
  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDragEnabled |
                 Qt::ItemIsSelectable);
  UpdatePodcastText(item, unlistened_count);

  // Load the podcast's image if it has one
  if (podcast.ImageUrlSmall().isValid()) {
    icon_loader_->LoadIcon(podcast.ImageUrlSmall().toString(), QString(), item);
  }

  podcasts_by_database_id_[podcast.database_id()] = item;

  return item;
}

QStandardItem* PodcastService::CreatePodcastEpisodeItem(
    const PodcastEpisode& episode) {
  QStandardItem* item = new QStandardItem;
  item->setText(episode.title().simplified());
  item->setData(Type_Episode, InternetModel::Role_Type);
  item->setData(QVariant::fromValue(episode), Role_Episode);
  item->setData(InternetModel::PlayBehaviour_UseSongLoader,
                InternetModel::Role_PlayBehaviour);
  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDragEnabled |
                 Qt::ItemIsSelectable);

  UpdateEpisodeText(item);

  episodes_by_database_id_[episode.database_id()] = item;

  return item;
}

void PodcastService::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addAction(IconLoader::Load("list-add", IconLoader::Base),
                             tr("Add podcast..."), this, SLOT(AddPodcast()));
    context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base),
                             tr("Update all podcasts"), app_->podcast_updater(),
                             SLOT(UpdateAllPodcastsNow()));

    context_menu_->addSeparator();
    context_menu_->addActions(GetPlaylistActions());

    context_menu_->addSeparator();
    update_selected_action_ = context_menu_->addAction(
        IconLoader::Load("view-refresh", IconLoader::Base),
        tr("Update this podcast"), this, SLOT(UpdateSelectedPodcast()));
    download_selected_action_ =
        context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                                 "", this, SLOT(DownloadSelectedEpisode()));
    info_selected_action_ =
        context_menu_->addAction(IconLoader::Load("about-info",
                                                  IconLoader::Base),
                                 tr("Podcast information"), this,
                                 SLOT(PodcastInfo()));
    delete_downloaded_action_ = context_menu_->addAction(
        IconLoader::Load("edit-delete", IconLoader::Base),
        tr("Delete downloaded data"), this, SLOT(DeleteDownloadedData()));
    copy_to_device_ = context_menu_->addAction(
        IconLoader::Load("multimedia-player-ipod-mini-blue", IconLoader::Base),
        tr("Copy to device..."), this, SLOT(CopyToDevice()));
    cancel_download_ = context_menu_->addAction(IconLoader::Load("cancel",
                                                IconLoader::Base),
                                                tr("Cancel download"), this,
                                                SLOT(CancelDownload()));
    remove_selected_action_ = context_menu_->addAction(
        IconLoader::Load("list-remove", IconLoader::Base), tr("Unsubscribe"),
        this, SLOT(RemoveSelectedPodcast()));

    context_menu_->addSeparator();
    set_new_action_ =
        context_menu_->addAction(tr("Mark as new"), this, SLOT(SetNew()));
    set_listened_action_ = context_menu_->addAction(tr("Mark as listened"),
                                                    this, SLOT(SetListened()));

    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base),
                             tr("Configure podcasts..."), this,
                             SLOT(ShowConfig()));

    copy_to_device_->setDisabled(
        app_->device_manager()->connected_devices_model()->rowCount() == 0);
    connect(app_->device_manager()->connected_devices_model(),
            SIGNAL(IsEmptyChanged(bool)), copy_to_device_,
            SLOT(setDisabled(bool)));
  }

  selected_episodes_.clear();
  selected_podcasts_.clear();
  explicitly_selected_podcasts_.clear();
  QSet<int> podcast_ids;

  for (const QModelIndex& index : model()->selected_indexes()) {
    switch (index.data(InternetModel::Role_Type).toInt()) {
      case Type_Podcast: {
        const int id = index.data(Role_Podcast).value<Podcast>().database_id();
        if (!podcast_ids.contains(id)) {
          selected_podcasts_.append(index);
          explicitly_selected_podcasts_.append(index);
          podcast_ids.insert(id);
        }
        break;
      }

      case Type_Episode: {
        selected_episodes_.append(index);

        // Add the parent podcast as well.
        const QModelIndex parent = index.parent();
        const int id = parent.data(Role_Podcast).value<Podcast>().database_id();
        if (!podcast_ids.contains(id)) {
          selected_podcasts_.append(parent);
          podcast_ids.insert(id);
        }
        break;
      }
    }
  }

  const bool episodes = !selected_episodes_.isEmpty();
  const bool podcasts = !selected_podcasts_.isEmpty();

  update_selected_action_->setEnabled(podcasts);
  remove_selected_action_->setEnabled(podcasts);
  set_new_action_->setEnabled(episodes || podcasts);
  set_listened_action_->setEnabled(episodes || podcasts);
  cancel_download_->setEnabled(episodes || podcasts);

  if (selected_episodes_.count() == 1) {
    const PodcastEpisode episode =
        selected_episodes_[0].data(Role_Episode).value<PodcastEpisode>();
    const bool downloaded = episode.downloaded();
    const bool listened = episode.listened();

    download_selected_action_->setEnabled(!downloaded);
    delete_downloaded_action_->setEnabled(downloaded);

    if (explicitly_selected_podcasts_.isEmpty()) {
      set_new_action_->setEnabled(listened);
      set_listened_action_->setEnabled(!listened || !episode.listened_date().isValid());
    }
  } else {
    download_selected_action_->setEnabled(episodes);
    delete_downloaded_action_->setEnabled(episodes);
  }

  if (selected_podcasts_.count() == 1) {
    if (selected_episodes_.count() == 1) {
      info_selected_action_->setText(tr("Episode information"));
      info_selected_action_->setEnabled(true);
    } else {
      info_selected_action_->setText(tr("Podcast information"));
      info_selected_action_->setEnabled(true);
    }
  } else {
    info_selected_action_->setText(tr("Podcast information"));
    info_selected_action_->setEnabled(false);
  }

  if (explicitly_selected_podcasts_.isEmpty() && selected_episodes_.isEmpty()) {
    PodcastEpisodeList epis = backend_->GetNewDownloadedEpisodes();
    set_listened_action_->setEnabled(!epis.isEmpty());
  }

  if (selected_episodes_.count() > 1) {
    download_selected_action_->setText(
        tr("Download %n episodes", "", selected_episodes_.count()));
  } else {
    download_selected_action_->setText(tr("Download this episode"));
  }

  GetAppendToPlaylistAction()->setEnabled(episodes || podcasts);
  GetReplacePlaylistAction()->setEnabled(episodes || podcasts);
  GetOpenInNewPlaylistAction()->setEnabled(episodes || podcasts);

  context_menu_->popup(global_pos);
}

void PodcastService::UpdateSelectedPodcast() {
  for (const QModelIndex& index : selected_podcasts_) {
    app_->podcast_updater()->UpdatePodcastNow(
        index.data(Role_Podcast).value<Podcast>());
  }
}

void PodcastService::RemoveSelectedPodcast() {
  for (const QModelIndex& index : selected_podcasts_) {
    backend_->Unsubscribe(index.data(Role_Podcast).value<Podcast>());
  }
}

void PodcastService::ReloadSettings() {
  InitialLoadSettings();
  ClearPodcastList(model_->invisibleRootItem());
  PopulatePodcastList(model_->invisibleRootItem());
}

void PodcastService::InitialLoadSettings() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);
  use_pretty_covers_ = s.value("pretty_covers", true).toBool();
  s.endGroup();
  s.beginGroup(kSettingsGroup);
  hide_listened_ = s.value("hide_listened", false).toBool();
  show_episodes_ = s.value("show_episodes", 0).toInt();
  s.endGroup();
  // TODO(notme): reload the podcast icons that are already loaded?
}

void PodcastService::EnsureAddPodcastDialogCreated() {
  add_podcast_dialog_.reset(new AddPodcastDialog(app_));
}

void PodcastService::AddPodcast() {
  EnsureAddPodcastDialogCreated();
  add_podcast_dialog_->show();
}

void PodcastService::FileCopied(int database_id) {
  SetListened(PodcastEpisodeList() << backend_->GetEpisodeById(database_id),
              true);
}

void PodcastService::SubscriptionAdded(const Podcast& podcast) {
  // Ensure the root item is lazy loaded already
  LazyLoadRoot();

  // The podcast might already be in the list - maybe the LazyLoadRoot() above
  // added it.
  QStandardItem* item = podcasts_by_database_id_[podcast.database_id()];
  if (!item) {
    item = CreatePodcastItem(podcast);
    model_->appendRow(item);
  }

  emit ScrollToIndex(MapToMergedModel(item->index()));
}

void PodcastService::SubscriptionRemoved(const Podcast& podcast) {
  QStandardItem* item = podcasts_by_database_id_.take(podcast.database_id());
  if (item) {
    // Remove any episode ID -> item mappings for the episodes in this podcast.
    for (int i = 0; i < item->rowCount(); ++i) {
      QStandardItem* episode_item = item->child(i);
      const int episode_id = episode_item->data(Role_Episode)
                                 .value<PodcastEpisode>()
                                 .database_id();

      episodes_by_database_id_.remove(episode_id);
    }

    // Remove this episode's row
    model_->removeRow(item->row());
  }
}

void PodcastService::EpisodesAdded(const PodcastEpisodeList& episodes) {
  QSet<int> seen_podcast_ids;

  for (const PodcastEpisode& episode : episodes) {
    const int database_id = episode.podcast_database_id();
    QStandardItem* parent = podcasts_by_database_id_[database_id];
    if (!parent) continue;

    parent->appendRow(CreatePodcastEpisodeItem(episode));
    if (!seen_podcast_ids.contains(database_id)) {
      // Update the unlistened count text once for each podcast
      int unlistened_count = 0;
      for (const PodcastEpisode& episode : backend_->GetEpisodes(database_id)) {
        if (!episode.listened()) {
          unlistened_count++;
        }
      }

      UpdatePodcastText(parent, unlistened_count);
      seen_podcast_ids.insert(database_id);
    }
    const Podcast podcast = parent->data(Role_Podcast).value<Podcast>();
    ReloadPodcast(podcast);
  }
}

void PodcastService::EpisodesUpdated(const PodcastEpisodeList& episodes) {
  QSet<int> seen_podcast_ids;
  QMap<int, Podcast> podcasts_map;

  for (const PodcastEpisode& episode : episodes) {
    const int podcast_database_id = episode.podcast_database_id();
    QStandardItem* item = episodes_by_database_id_[episode.database_id()];
    QStandardItem* parent = podcasts_by_database_id_[podcast_database_id];
    if (!item || !parent) continue;
    // Update the episode data on the item, and update the item's text.
    item->setData(QVariant::fromValue(episode), Role_Episode);
    UpdateEpisodeText(item);

    // Update the parent podcast's text too.
    if (!seen_podcast_ids.contains(podcast_database_id)) {
      // Update the unlistened count text once for each podcast
      int unlistened_count = 0;
      for (const PodcastEpisode& episode :
           backend_->GetEpisodes(podcast_database_id)) {
        if (!episode.listened()) {
          unlistened_count++;
        }
      }

      UpdatePodcastText(parent, unlistened_count);
      seen_podcast_ids.insert(podcast_database_id);
    }
    const Podcast podcast = parent->data(Role_Podcast).value<Podcast>();
    podcasts_map[podcast.database_id()] = podcast;
  }
  for (const Podcast& podcast_tmp : podcasts_map.values()) {
    ReloadPodcast(podcast_tmp);
  }
}

void PodcastService::DownloadSelectedEpisode() {
  for (const QModelIndex& index : selected_episodes_) {
    app_->podcast_downloader()->DownloadEpisode(
        index.data(Role_Episode).value<PodcastEpisode>());
  }
}

void PodcastService::PodcastInfo() {
  if (selected_podcasts_.isEmpty()) {
    // Should never happen.
    return;
  }
  const Podcast podcast =
      selected_podcasts_[0].data(Role_Podcast).value<Podcast>();
  podcast_info_dialog_.reset(new PodcastInfoDialog(app_));

  if (selected_episodes_.count() == 1) {
    const PodcastEpisode episode =
        selected_episodes_[0].data(Role_Episode).value<PodcastEpisode>();
    podcast_info_dialog_->ShowEpisode(episode, podcast);
  } else {
    podcast_info_dialog_->ShowPodcast(podcast);
  }
}

void PodcastService::DeleteDownloadedData() {
  for (const QModelIndex& index : selected_episodes_) {
    app_->podcast_deleter()->DeleteEpisode(
        index.data(Role_Episode).value<PodcastEpisode>());
  }
}

void PodcastService::DownloadProgressChanged(const PodcastEpisode& episode,
                                             PodcastDownload::State state,
                                             int percent) {
  QStandardItem* item = episodes_by_database_id_[episode.database_id()];
  QStandardItem* item2 = podcasts_by_database_id_[episode.podcast_database_id()];
  if (!item || !item2) return;

  UpdateEpisodeText(item, state, percent);
  UpdatePodcastText(item2, state, percent);
}

void PodcastService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Podcasts);
}

void PodcastService::CurrentSongChanged(const Song& metadata) {
  // This does two db queries, and we are called on every song change, so run
  // this off the main thread.
  QtConcurrent::run(this, &PodcastService::UpdatePodcastListenedStateAsync,
                    metadata);
}

void PodcastService::UpdatePodcastListenedStateAsync(const Song& metadata) {
  // Check whether this song is one of our podcast episodes.
  PodcastEpisode episode = backend_->GetEpisodeByUrlOrLocalUrl(metadata.url());
  if (!episode.is_valid()) return;

  // Mark it as listened if it's not already
  if (!episode.listened() || !episode.listened_date().isValid()) {
    episode.set_listened(true);
    episode.set_listened_date(QDateTime::currentDateTime());
    backend_->UpdateEpisodes(PodcastEpisodeList() << episode);
  }
}

void PodcastService::SetNew() {
  SetListened(selected_episodes_, explicitly_selected_podcasts_, false);
}

void PodcastService::SetListened() {
  if (selected_episodes_.isEmpty() && explicitly_selected_podcasts_.isEmpty())
    SetListened(backend_->GetNewDownloadedEpisodes(), true);
  else
    SetListened(selected_episodes_, explicitly_selected_podcasts_, true);
}

void PodcastService::SetListened(const PodcastEpisodeList& episodes_list,
                                 bool listened) {
  PodcastEpisodeList episodes;
  QDateTime current_date_time = QDateTime::currentDateTime();
  for (PodcastEpisode episode : episodes_list) {
    episode.set_listened(listened);
    if (listened) {
      episode.set_listened_date(current_date_time);
    }
    episodes << episode;
  }

  backend_->UpdateEpisodes(episodes);
}

void PodcastService::SetListened(const QModelIndexList& episode_indexes,
                                 const QModelIndexList& podcast_indexes,
                                 bool listened) {
  PodcastEpisodeList episodes;

  // Get all the episodes from the indexes.
  for (const QModelIndex& index : episode_indexes) {
    episodes << index.data(Role_Episode).value<PodcastEpisode>();
  }

  for (const QModelIndex& podcast : podcast_indexes) {
    for (int i = 0; i < podcast.model()->rowCount(podcast); ++i) {
      const QModelIndex& index = podcast.child(i, 0);
      episodes << index.data(Role_Episode).value<PodcastEpisode>();
    }
  }

  // Update each one with the new state and maybe the listened time.
  QDateTime current_date_time = QDateTime::currentDateTime();
  for (int i = 0; i < episodes.count(); ++i) {
    PodcastEpisode* episode = &episodes[i];
    episode->set_listened(listened);
    if (listened) {
      episode->set_listened_date(current_date_time);
    }
  }

  backend_->UpdateEpisodes(episodes);
}

QModelIndex PodcastService::MapToMergedModel(const QModelIndex& index) const {
  return model()->merged_model()->mapFromSource(proxy_->mapFromSource(index));
}

void PodcastService::LazyLoadRoot() {
  if (root_->data(InternetModel::Role_CanLazyLoad).toBool()) {
    root_->setData(false, InternetModel::Role_CanLazyLoad);
    LazyPopulate(root_);
  }
}

void PodcastService::SubscribeAndShow(const QVariant& podcast_or_opml) {
  if (podcast_or_opml.canConvert<Podcast>()) {
    Podcast podcast(podcast_or_opml.value<Podcast>());
    backend_->Subscribe(&podcast);

    // Lazy load the root item if it hasn't been already
    LazyLoadRoot();

    QStandardItem* item = podcasts_by_database_id_[podcast.database_id()];
    if (item) {
      // There will be an item already if this podcast was already there,
      // otherwise it'll be scrolled to when the item is created.
      emit ScrollToIndex(MapToMergedModel(item->index()));
    }
  } else if (podcast_or_opml.canConvert<OpmlContainer>()) {
    EnsureAddPodcastDialogCreated();

    add_podcast_dialog_->ShowWithOpml(podcast_or_opml.value<OpmlContainer>());
  }
}

void PodcastService::ReloadPodcast(const Podcast& podcast) {
  if (!(hide_listened_ || (show_episodes_ > 0))) {
    return;
  }
  QStandardItem* item = podcasts_by_database_id_[podcast.database_id()];

  model_->invisibleRootItem()->removeRow(item->row());
  model_->invisibleRootItem()->appendRow(CreatePodcastItem(podcast));
}
