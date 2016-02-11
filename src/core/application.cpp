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

#ifdef HAVE_LIBLASTFM
#include "internet/lastfm/lastfmservice.h"
#endif  // HAVE_LIBLASTFM

#ifdef HAVE_MOODBAR
#include "moodbar/moodbarcontroller.h"
#include "moodbar/moodbarloader.h"
#endif

bool Application::kIsPortable = false;

Application::Application(QObject* parent)
    : QObject(parent),
      tag_reader_client_([&](){
        TagReaderClient* client = new TagReaderClient(this);
        MoveToNewThread(client);
        client->Start();
        return client;
      }),
      database_([&]() {
        Database* db = new Database(this, this);
        MoveToNewThread(db);
        DoInAMinuteOrSo(db, SLOT(DoBackup()));
        return db;
      }),
      album_cover_loader_([&]() {
        AlbumCoverLoader* loader = new AlbumCoverLoader(this);
        MoveToNewThread(loader);
        return loader;
      }),
      playlist_backend_([&]() {
        PlaylistBackend* backend = new PlaylistBackend(this, this);
        MoveToThread(backend, database_->thread());
        return backend;
      }),
      podcast_backend_([&]() {
        PodcastBackend* backend = new PodcastBackend(this, this);
        MoveToThread(backend, database_->thread());
        return backend;
      }),
      appearance_([=]() { return new Appearance(this); }),
      cover_providers_([=]() { return new CoverProviders(this); }),
      task_manager_([=]() { return new TaskManager(this); }),
      player_([=]() { return new Player(this, this); }),
      playlist_manager_([=]() { return new PlaylistManager(this); }),
      current_art_loader_([=]() { return new CurrentArtLoader(this, this); }),
      global_search_([=]() { return new GlobalSearch(this, this); }),
      internet_model_([=]() { return new InternetModel(this, this); }),
      library_([=]() { return new Library(this, this); }),
      device_manager_([=]() { return new DeviceManager(this, this); }),
      podcast_updater_([=]() { return new PodcastUpdater(this, this); }),
      podcast_deleter_([=]() {
        PodcastDeleter* deleter = new PodcastDeleter(this, this);
        MoveToNewThread(deleter);
        return deleter;
      }),
      podcast_downloader_([=]() {
        return new PodcastDownloader(this, this);
      }),
      gpodder_sync_([=]() { return new GPodderSync(this, this); }),
      moodbar_loader_([=]() {
#ifdef HAVE_MOODBAR
        return new MoodbarLoader(this, this);
#else
        return nullptr;
#endif
      }),
      moodbar_controller_([=]() {
#ifdef HAVE_MOODBAR
        return new MoodbarController(this, this);
#else
        return nullptr;
#endif
      }),
      network_remote_([=]() {
        NetworkRemote* remote = new NetworkRemote(this);
        MoveToNewThread(remote);
        return remote;
      }),
      network_remote_helper_([=]() {
        return new NetworkRemoteHelper(this);
      }),
      scrobbler_([=]() {
#ifdef HAVE_LIBLASTFM
        return new LastFMService(this, this);
#else
        return nullptr;
#endif
      }) {
  // This must be before library_->Init();
  // In the constructor the helper waits for the signal
  // PlaylistManagerInitialized
  // to start the remote. Without the playlist manager clementine can
  // crash when a client connects before the manager is initialized!
  network_remote_helper_.get();
  library_->Init();
}

Application::~Application() {
  // It's important that the device manager is deleted before the database.
  // Deleting the database deletes all objects that have been created in its
  // thread, including some device library backends.
  device_manager_.reset();

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

LibraryBackend* Application::library_backend() const {
  return library()->backend();
}

LibraryModel* Application::library_model() const { return library()->model(); }

void Application::ReloadSettings() { emit SettingsChanged(); }

void Application::OpenSettingsDialogAtPage(SettingsDialog::Page page) {
  emit SettingsDialogRequested(page);
}
