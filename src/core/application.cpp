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
#include "appearance.h"
#include "config.h"
#include "database.h"
#include "player.h"
#include "tagreaderclient.h"
#include <QWebFrame>
#include "taskmanager.h"
#include "covers/albumcoverloader.h"
#include "covers/coverproviders.h"
#include "covers/currentartloader.h"
#include "devices/devicemanager.h"
#include "internet/core/internetmodel.h"
#include "globalsearch/globalsearch.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "networkremote/clementinewebpage.h"
#include "networkremote/networkremote.h"
#include "networkremote/networkremotehelper.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlistmanager.h"
#include "internet/podcasts/gpoddersync.h"
#include "internet/podcasts/podcastbackend.h"
#include "internet/podcasts/podcastdeleter.h"
#include "internet/podcasts/podcastdownloader.h"
#include "internet/podcasts/podcastupdater.h"

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
      tag_reader_client_(nullptr),
      database_(nullptr),
      album_cover_loader_(nullptr),
      playlist_backend_(nullptr),
      podcast_backend_(nullptr),
      appearance_(nullptr),
      cover_providers_(nullptr),
      task_manager_(nullptr),
      player_(nullptr),
      playlist_manager_(nullptr),
      current_art_loader_(nullptr),
      global_search_(nullptr),
      internet_model_(nullptr),
      library_(nullptr),
      device_manager_(nullptr),
      podcast_updater_(nullptr),
      podcast_deleter_(nullptr),
      podcast_downloader_(nullptr),
      gpodder_sync_(nullptr),
      moodbar_loader_(nullptr),
      moodbar_controller_(nullptr),
      network_remote_(nullptr),
      network_remote_helper_(nullptr),
      scrobbler_(nullptr) {
}

Application::~Application() {
  // It's important that the device manager is deleted before the database.
  // Deleting the database deletes all objects that have been created in its
  // thread, including some device library backends.
  delete device_manager_;
  device_manager_ = nullptr;

  for (QObject* object : objects_in_threads_) {
    object->deleteLater();
  }

  for (QThread* thread : threads_) {
    thread->quit();
  }

  for (QThread* thread : threads_) {
    thread->wait();
  }
}

void Application::Init() {
  tag_reader_client_ = new TagReaderClient(this);
  MoveToNewThread(tag_reader_client_);
  tag_reader_client_->Start();

  database_ = new Database(this, this);
  MoveToNewThread(database_);

  album_cover_loader_ = new AlbumCoverLoader(this);
  MoveToNewThread(album_cover_loader_);

  playlist_backend_ = new PlaylistBackend(this, this);
  MoveToThread(playlist_backend_, database_->thread());

  podcast_backend_ = new PodcastBackend(this, this);
  MoveToThread(podcast_backend_, database_->thread());

  appearance_ = new Appearance(this);
  cover_providers_ = new CoverProviders(this);
  task_manager_ = new TaskManager(this);
  player_ = new Player(this, this);
  playlist_manager_ = new PlaylistManager(this, this);
  current_art_loader_ = new CurrentArtLoader(this, this);
  global_search_ = new GlobalSearch(this, this);
  internet_model_ = new InternetModel(this, this);
  library_ = new Library(this, this);
  device_manager_ = new DeviceManager(this, this);
  podcast_updater_ = new PodcastUpdater(this, this);

  podcast_deleter_ = new PodcastDeleter(this, this);
  MoveToNewThread(podcast_deleter_);

  podcast_downloader_ = new PodcastDownloader(this, this);
  gpodder_sync_ = new GPodderSync(this, this);

#ifdef HAVE_MOODBAR
  moodbar_loader_ = new MoodbarLoader(this, this);
  moodbar_controller_ = new MoodbarController(this, this);
#endif

  // Network Remote
  ClementineWebPage* web_channel = new ClementineWebPage(this);
  network_remote_ = new NetworkRemote(web_channel, this);
  MoveToNewThread(network_remote_);

  // This must be before libraray_->Init();
  // In the constructor the helper waits for the signal
  // PlaylistManagerInitialized
  // to start the remote. Without the playlist manager clementine can
  // crash when a client connects before the manager is initialized!
  network_remote_helper_ = new NetworkRemoteHelper(this);

#ifdef HAVE_LIBLASTFM
  scrobbler_ = new LastFMService(this, this);
#endif  // HAVE_LIBLASTFM

  library_->Init();

  DoInAMinuteOrSo(database_, SLOT(DoBackup()));
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
  objects_in_threads_ << object;
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
