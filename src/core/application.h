/* This file is part of Clementine.
   Copyright 2012-2013, Andreas <asfa194@gmail.com>
   Copyright 2012-2013, David Sansome <me@davidsansome.com>
   Copyright 2013, Arnaud Bienner <arnaud.bienner@gmail.com>
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

#ifndef CORE_APPLICATION_H_
#define CORE_APPLICATION_H_

#include "config.h"
#include "core/appearance.h"
#include "core/database.h"
#include "core/lazy.h"
#include "core/player.h"
#include "covers/albumcoverloader.h"
#include "covers/coverproviders.h"
#include "covers/currentartloader.h"
#include "devices/devicemanager.h"
#include "globalsearch/globalsearch.h"
#include "internet/core/internetmodel.h"
#include "internet/core/scrobbler.h"
#include "internet/podcasts/gpoddersync.h"
#include "internet/podcasts/podcastbackend.h"
#include "internet/podcasts/podcastdeleter.h"
#include "internet/podcasts/podcastdownloader.h"
#include "internet/podcasts/podcastupdater.h"
#include "library/librarybackend.h"
#include "library/library.h"
#include "moodbar/moodbarcontroller.h"
#include "moodbar/moodbarloader.h"
#include "networkremote/networkremote.h"
#include "networkremote/networkremotehelper.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlistmanager.h"
#include "tagreaderclient.h"
#include "taskmanager.h"
#include "ui/settingsdialog.h"

#include <QObject>

class Application : public QObject {
  Q_OBJECT

 public:
  static bool kIsPortable;

  explicit Application(QObject* parent = nullptr);
  ~Application();

  const QString& language_name() const { return language_name_; }
  // Same as language_name, but remove the region code at the end if there is
  // one
  QString language_without_region() const;
  void set_language_name(const QString& name) { language_name_ = name; }

  TagReaderClient* tag_reader_client() const {
    return tag_reader_client_.get();
  }
  Database* database() const { return database_.get(); }
  AlbumCoverLoader* album_cover_loader() const {
    return album_cover_loader_.get();
  }
  PlaylistBackend* playlist_backend() const { return playlist_backend_.get(); }
  PodcastBackend* podcast_backend() const { return podcast_backend_.get(); }
  Appearance* appearance() const { return appearance_.get(); }
  CoverProviders* cover_providers() const { return cover_providers_.get(); }
  TaskManager* task_manager() const { return task_manager_.get(); }
  Player* player() const { return player_.get(); }
  PlaylistManager* playlist_manager() const { return playlist_manager_.get(); }
  CurrentArtLoader* current_art_loader() const {
    return current_art_loader_.get();
  }
  GlobalSearch* global_search() const { return global_search_.get(); }
  InternetModel* internet_model() const { return internet_model_.get(); }
  Library* library() const { return library_.get(); }
  DeviceManager* device_manager() const { return device_manager_.get(); }
  PodcastUpdater* podcast_updater() const { return podcast_updater_.get(); }
  PodcastDeleter* podcast_deleter() const { return podcast_deleter_.get(); }
  PodcastDownloader* podcast_downloader() const {
    return podcast_downloader_.get();
  }
  GPodderSync* gpodder_sync() const { return gpodder_sync_.get(); }
  MoodbarLoader* moodbar_loader() const { return moodbar_loader_.get(); }
  MoodbarController* moodbar_controller() const {
    return moodbar_controller_.get();
  }
  NetworkRemote* network_remote() const { return network_remote_.get(); }
  NetworkRemoteHelper* network_remote_helper() const {
    return network_remote_helper_.get();
  }
  Scrobbler* scrobbler() const { return scrobbler_.get(); }

  LibraryBackend* library_backend() const;
  LibraryModel* library_model() const;

  void MoveToNewThread(QObject* object);
  void MoveToThread(QObject* object, QThread* thread);

 public slots:
  void AddError(const QString& message);
  void ReloadSettings();
  void OpenSettingsDialogAtPage(SettingsDialog::Page page);

signals:
  void ErrorAdded(const QString& message);
  void SettingsChanged();
  void SettingsDialogRequested(SettingsDialog::Page page);

 private:
  QString language_name_;

  Lazy<TagReaderClient> tag_reader_client_;
  Lazy<Database> database_;
  Lazy<AlbumCoverLoader> album_cover_loader_;
  Lazy<PlaylistBackend> playlist_backend_;
  Lazy<PodcastBackend> podcast_backend_;
  Lazy<Appearance> appearance_;
  Lazy<CoverProviders> cover_providers_;
  Lazy<TaskManager> task_manager_;
  Lazy<Player> player_;
  Lazy<PlaylistManager> playlist_manager_;
  Lazy<CurrentArtLoader> current_art_loader_;
  Lazy<GlobalSearch> global_search_;
  Lazy<InternetModel> internet_model_;
  Lazy<Library> library_;
  Lazy<DeviceManager> device_manager_;
  Lazy<PodcastUpdater> podcast_updater_;
  Lazy<PodcastDeleter> podcast_deleter_;
  Lazy<PodcastDownloader> podcast_downloader_;
  Lazy<GPodderSync> gpodder_sync_;
  Lazy<MoodbarLoader> moodbar_loader_;
  Lazy<MoodbarController> moodbar_controller_;
  Lazy<NetworkRemote> network_remote_;
  Lazy<NetworkRemoteHelper> network_remote_helper_;
  Lazy<Scrobbler> scrobbler_;

  QList<QObject*> objects_in_threads_;
  QList<QThread*> threads_;
};

#endif  // CORE_APPLICATION_H_
