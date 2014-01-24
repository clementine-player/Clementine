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

#include "application.h"
#include "appearance.h"
#include "config.h"
#include "database.h"
#include "player.h"
#include "tagreaderclient.h"
#include "taskmanager.h"
#include "covers/albumcoverloader.h"
#include "covers/coverproviders.h"
#include "covers/currentartloader.h"
#include "devices/devicemanager.h"
#include "internet/internetmodel.h"
#include "globalsearch/globalsearch.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "networkremote/networkremote.h"
#include "networkremote/networkremotehelper.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlistmanager.h"
#include "podcasts/gpoddersync.h"
#include "podcasts/podcastbackend.h"
#include "podcasts/podcastdownloader.h"
#include "podcasts/podcastupdater.h"

#ifdef HAVE_MOODBAR
# include "moodbar/moodbarcontroller.h"
# include "moodbar/moodbarloader.h"
#endif

bool Application::kIsPortable = false;

Application::Application(QObject* parent)
  : QObject(parent),
    tag_reader_client_(NULL),
    database_(NULL),
    album_cover_loader_(NULL),
    playlist_backend_(NULL),
    podcast_backend_(NULL),
    appearance_(NULL),
    cover_providers_(NULL),
    task_manager_(NULL),
    player_(NULL),
    playlist_manager_(NULL),
    current_art_loader_(NULL),
    global_search_(NULL),
    internet_model_(NULL),
    library_(NULL),
    device_manager_(NULL),
    podcast_updater_(NULL),
    podcast_downloader_(NULL),
    gpodder_sync_(NULL),
    moodbar_loader_(NULL),
    moodbar_controller_(NULL),
    network_remote_(NULL),
    network_remote_helper_(NULL)
{
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
  podcast_downloader_ = new PodcastDownloader(this, this);
  gpodder_sync_ = new GPodderSync(this, this);

#ifdef HAVE_MOODBAR
  moodbar_loader_ = new MoodbarLoader(this, this);
  moodbar_controller_ = new MoodbarController(this, this);
#endif

  // Network Remote
  network_remote_ = new NetworkRemote(this);
  MoveToNewThread(network_remote_);

  // This must be before libraray_->Init();
  // In the constructor the helper waits for the signal PlaylistManagerInitialized
  // to start the remote. Without the playlist manager clementine can
  // crash when a client connects before the manager is initialized!
  network_remote_helper_ = new NetworkRemoteHelper(this);

  library_->Init();

  DoInAMinuteOrSo(database_, SLOT(DoBackup()));
}

Application::~Application() {
  // It's important that the device manager is deleted before the database.
  // Deleting the database deletes all objects that have been created in its
  // thread, including some device library backends.
  delete device_manager_; device_manager_ = NULL;

  foreach (QObject* object, objects_in_threads_) {
    object->deleteLater();
  }

  foreach (QThread* thread, threads_) {
    thread->quit();
  }

  foreach (QThread* thread, threads_) {
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
  object->setParent(NULL);
  object->moveToThread(thread);
  objects_in_threads_ << object;
}

void Application::AddError(const QString& message) {
  emit ErrorAdded(message);
}

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

LibraryModel* Application::library_model() const {
  return library()->model();
}

void Application::ReloadSettings() {
  emit SettingsChanged();
}

void Application::OpenSettingsDialogAtPage(SettingsDialog::Page page) {
  emit SettingsDialogRequested(page);
}
