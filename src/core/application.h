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

#include <memory>

#include <QObject>

#include "ui/settingsdialog.h"

class QSettings;

class AlbumCoverLoader;
class Appearance;
class ApplicationImpl;
class CoverProviders;
class CurrentArtLoader;
class Database;
class DeviceManager;
class GlobalSearch;
class GPodderSync;
class InternetModel;
class Library;
class LibraryBackend;
class LibraryModel;
class MoodbarController;
class MoodbarLoader;
class NetworkRemote;
class NetworkRemoteHelper;
class Player;
class PlaylistBackend;
class PlaylistManager;
class PodcastBackend;
class PodcastDeleter;
class PodcastDownloader;
class PodcastUpdater;
class Scrobbler;
class TagReaderClient;
class TaskManager;

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

  AlbumCoverLoader* album_cover_loader() const;
  Appearance* appearance() const;
  CoverProviders* cover_providers() const;
  CurrentArtLoader* current_art_loader() const;
  Database* database() const;
  DeviceManager* device_manager() const;
  GlobalSearch* global_search() const;
  GPodderSync* gpodder_sync() const;
  InternetModel* internet_model() const;
  Library* library() const;
  LibraryBackend* library_backend() const;
  LibraryModel* library_model() const;
  MoodbarController* moodbar_controller() const;
  MoodbarLoader* moodbar_loader() const;
  NetworkRemoteHelper* network_remote_helper() const;
  NetworkRemote* network_remote() const;
  Player* player() const;
  PlaylistBackend* playlist_backend() const;
  PlaylistManager* playlist_manager() const;
  PodcastBackend* podcast_backend() const;
  PodcastDeleter* podcast_deleter() const;
  PodcastDownloader* podcast_downloader() const;
  PodcastUpdater* podcast_updater() const;
  Scrobbler* scrobbler() const;
  TagReaderClient* tag_reader_client() const;
  TaskManager* task_manager() const;

  void DirtySettings();

  void MoveToNewThread(QObject* object);
  void MoveToThread(QObject* object, QThread* thread);

 public slots:
  void AddError(const QString& message);
  void ReloadSettings();
  void OpenSettingsDialogAtPage(SettingsDialog::Page page);

signals:
  void ErrorAdded(const QString& message);
  void SettingsChanged();
  void SaveSettings(QSettings* settings);
  void SettingsDialogRequested(SettingsDialog::Page page);

 private slots:
  void SaveSettings_();

 private:
  QString language_name_;
  std::unique_ptr<ApplicationImpl> p_;
  QList<QThread*> threads_;
};

#endif  // CORE_APPLICATION_H_
