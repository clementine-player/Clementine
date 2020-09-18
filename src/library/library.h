/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <QHash>
#include <QObject>
#include <QUrl>
#include <memory>

#include "core/song.h"

class Application;
class Database;
class LibraryBackend;
class LibraryModel;
class LibraryDirectoryModel;
class LibraryWatcher;
class TaskManager;
class Thread;

class Library : public QObject {
  Q_OBJECT

 public:
  Library(Application* app, QObject* parent);
  ~Library();

  static const char* kSongsTable;
  static const char* kDirsTable;
  static const char* kSubdirsTable;
  static const char* kFtsTable;

  void Init();

  LibraryBackend* backend() const { return backend_.get(); }
  LibraryModel* model() const { return model_; }
  LibraryDirectoryModel* directory_model() const { return dir_model_; }

  QString full_rescan_reason(int schema_version) const {
    return full_rescan_revisions_.value(schema_version, QString());
  }

  void WriteAllSongsStatisticsToFiles();

 public slots:
  void ReloadSettings();

  void PauseWatcher();
  void ResumeWatcher();

  void FullScan();

 private slots:
  void IncrementalScan();

  void SongsStatisticsChanged(const SongList& songs);
  void SongsRatingChanged(const SongList& songs);
  void CurrentSongChanged(const Song& song);
  void Stopped();

 private:
  SongList FilterCurrentWMASong(SongList songs, Song* queued);

 private:
  Application* app_;
  std::shared_ptr<LibraryBackend> backend_;
  LibraryModel* model_;
  LibraryDirectoryModel* dir_model_;

  LibraryWatcher* watcher_;
  Thread* watcher_thread_;

  bool save_statistics_in_files_;
  bool save_ratings_in_files_;

  // Hack: Gstreamer doesn't cope well with WMA files being rewritten while
  // being played, so we delay statistics and rating changes until the current
  // song has finished playing.
  QUrl current_wma_song_url_;
  Song queued_statistics_;
  Song queued_rating_;

  // DB schema versions which should trigger a full library rescan (each of
  // those with a short reason why).
  QHash<int, QString> full_rescan_revisions_;
};

#endif
