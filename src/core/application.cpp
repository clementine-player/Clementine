/* This file is part of Clementine.
   Copyright 2012-2013, Andreas <asfa194@gmail.com>
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, Marti Raudsepp <marti@juffo.org>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "application.h"

#include "config.h"
#include "core/appearance.h"
#include "core/database.h"
#include "core/lazy.h"
#include "core/player.h"
#include "core/tagreaderclient.h"
#include "core/taskmanager.h"
#include "covers/albumcoverloader.h"
#include "covers/coverproviders.h"
#include "covers/currentartloader.h"
#include "covers/musicbrainzcoverprovider.h"
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

#ifdef HAVE_LIBLASTFM
#include "covers/lastfmcoverprovider.h"
#include "internet/lastfm/lastfmservice.h"
#endif  // HAVE_LIBLASTFM

#ifdef HAVE_MOODBAR
#include "moodbar/moodbarcontroller.h"
#include "moodbar/moodbarloader.h"
#endif

bool Application::kIsPortable = false;

class ApplicationImpl {
 public:
  ApplicationImpl(Application* app)
      : tag_reader_client_([=]() {
          TagReaderClient* client = new TagReaderClient(app);
          app->MoveToNewThread(client);
          client->Start();
          return client;
        }),
        database_([=]() {
          Database* db = new Database(app, app);
          app->MoveToNewThread(db);
          DoInAMinuteOrSo(db, SLOT(DoBackup()));
          return db;
        }),
        album_cover_loader_([=]() {
          AlbumCoverLoader* loader = new AlbumCoverLoader(app);
          app->MoveToNewThread(loader);
          return loader;
        }),
        playlist_backend_([=]() {
          PlaylistBackend* backend = new PlaylistBackend(app, app);
          app->MoveToThread(backend, database_->thread());
          return backend;
        }),
        podcast_backend_([=]() {
          PodcastBackend* backend = new PodcastBackend(app, app);
          app->MoveToThread(backend, database_->thread());
          return backend;
        }),
        appearance_([=]() { return new Appearance(app); }),
        cover_providers_([=]() {
          CoverProviders* cover_providers = new CoverProviders(app);
          // Initialize the repository of cover providers.
          cover_providers->AddProvider(new MusicbrainzCoverProvider);
        #ifdef HAVE_LIBLASTFM
          cover_providers->AddProvider(new LastFmCoverProvider(app));
        #endif
          return cover_providers;
        }),
        task_manager_([=]() { return new TaskManager(app); }),
        player_([=]() { return new Player(app, app); }),
        playlist_manager_([=]() { return new PlaylistManager(app); }),
        current_art_loader_([=]() { return new CurrentArtLoader(app, app); }),
        global_search_([=]() { return new GlobalSearch(app, app); }),
        internet_model_([=]() { return new InternetModel(app, app); }),
        library_([=]() { return new Library(app, app); }),
        device_manager_([=]() { return new DeviceManager(app, app); }),
        podcast_updater_([=]() { return new PodcastUpdater(app, app); }),
        podcast_deleter_([=]() {
          PodcastDeleter* deleter = new PodcastDeleter(app, app);
          app->MoveToNewThread(deleter);
          return deleter;
        }),
        podcast_downloader_([=]() { return new PodcastDownloader(app, app); }),
        gpodder_sync_([=]() { return new GPodderSync(app, app); }),
        moodbar_loader_([=]() {
#ifdef HAVE_MOODBAR
          return new MoodbarLoader(app, app);
#else
          return nullptr;
#endif
        }),
        moodbar_controller_([=]() {
#ifdef HAVE_MOODBAR
          return new MoodbarController(app, app);
#else
          return nullptr;
#endif
        }),
        network_remote_([=]() {
          NetworkRemote* remote = new NetworkRemote(app);
          app->MoveToNewThread(remote);
          return remote;
        }),
        network_remote_helper_([=]() { return new NetworkRemoteHelper(app); }),
        scrobbler_([=]() {
#ifdef HAVE_LIBLASTFM
          return new LastFMService(app, app);
#else
          return nullptr;
#endif
        }) {
  }

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
};

Application::Application(QObject* parent)
    : QObject(parent), p_(new ApplicationImpl(this)) {
  // This must be before library_->Init();
  // In the constructor the helper waits for the signal
  // PlaylistManagerInitialized
  // to start the remote. Without the playlist manager clementine can
  // crash when a client connects before the manager is initialized!
  network_remote_helper();
  library()->Init();

  // TODO(John Maguire): Make this not a weird singleton.
  tag_reader_client();
}

Application::~Application() {
  // It's important that the device manager is deleted before the database.
  // Deleting the database deletes all objects that have been created in its
  // thread, including some device library backends.
  p_->device_manager_.reset();

  for (QThread* thread : threads_) {
    thread->quit();
  }

  for (QThread* thread : threads_) {
    thread->wait();
  }
}

void Application::MoveToNewThread(QObject* object) {
  QThread* thread = new QThread(this);

  MoveToThread(object, thread);

  thread->start();
  threads_ << thread;
}

void Application::MoveToThread(QObject* object, QThread* thread) {
  object->setParent(nullptr);
  object->moveToThread(thread);
}

void Application::AddError(const QString& message) { emit ErrorAdded(message); }

QString Application::language_without_region() const {
  const int underscore = language_name_.indexOf('_');
  if (underscore != -1) {
    return language_name_.left(underscore);
  }
  return language_name_;
}

void Application::ReloadSettings() { emit SettingsChanged(); }

void Application::OpenSettingsDialogAtPage(SettingsDialog::Page page) {
  emit SettingsDialogRequested(page);
}

AlbumCoverLoader* Application::album_cover_loader() const {
  return p_->album_cover_loader_.get();
}

Appearance* Application::appearance() const { return p_->appearance_.get(); }

CoverProviders* Application::cover_providers() const {
  return p_->cover_providers_.get();
}

CurrentArtLoader* Application::current_art_loader() const {
  return p_->current_art_loader_.get();
}

Database* Application::database() const { return p_->database_.get(); }

DeviceManager* Application::device_manager() const {
  return p_->device_manager_.get();
}

GlobalSearch* Application::global_search() const {
  return p_->global_search_.get();
}

GPodderSync* Application::gpodder_sync() const {
  return p_->gpodder_sync_.get();
}

InternetModel* Application::internet_model() const {
  return p_->internet_model_.get();
}

Library* Application::library() const { return p_->library_.get(); }

LibraryBackend* Application::library_backend() const {
  return library()->backend();
}

LibraryModel* Application::library_model() const { return library()->model(); }

MoodbarController* Application::moodbar_controller() const {
  return p_->moodbar_controller_.get();
}

MoodbarLoader* Application::moodbar_loader() const {
  return p_->moodbar_loader_.get();
}

NetworkRemoteHelper* Application::network_remote_helper() const {
  return p_->network_remote_helper_.get();
}

NetworkRemote* Application::network_remote() const {
  return p_->network_remote_.get();
}

Player* Application::player() const { return p_->player_.get(); }

PlaylistBackend* Application::playlist_backend() const {
  return p_->playlist_backend_.get();
}

PlaylistManager* Application::playlist_manager() const {
  return p_->playlist_manager_.get();
}

PodcastBackend* Application::podcast_backend() const {
  return p_->podcast_backend_.get();
}

PodcastDeleter* Application::podcast_deleter() const {
  return p_->podcast_deleter_.get();
}

PodcastDownloader* Application::podcast_downloader() const {
  return p_->podcast_downloader_.get();
}

PodcastUpdater* Application::podcast_updater() const {
  return p_->podcast_updater_.get();
}

Scrobbler* Application::scrobbler() const { return p_->scrobbler_.get(); }

TagReaderClient* Application::tag_reader_client() const {
  return p_->tag_reader_client_.get();
}

TaskManager* Application::task_manager() const {
  return p_->task_manager_.get();
}
