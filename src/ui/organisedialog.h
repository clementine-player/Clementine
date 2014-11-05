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

#ifndef ORGANISEDIALOG_H
#define ORGANISEDIALOG_H

#include <memory>

#include <QDialog>
#include <QMap>
#include <QUrl>

#include "gtest/gtest_prod.h"

#include "core/organise.h"
#include "core/organiseformat.h"
#include "core/song.h"

class LibraryWatcher;
class OrganiseErrorDialog;
class TaskManager;
class Ui_OrganiseDialog;

class QAbstractItemModel;

class OrganiseDialog : public QDialog {
  Q_OBJECT

 public:
  OrganiseDialog(TaskManager* task_manager, QWidget* parent = nullptr);
  ~OrganiseDialog();

  static const char* kDefaultFormat;
  static const char* kSettingsGroup;

  QSize sizeHint() const;

  void SetDestinationModel(QAbstractItemModel* model, bool devices = false);

  // These functions return true if any songs were actually added to the dialog.
  // SetSongs returns immediately, SetUrls and SetFilenames load the songs in
  // the background.
  bool SetSongs(const SongList& songs);
  bool SetUrls(const QList<QUrl>& urls);
  bool SetFilenames(const QStringList& filenames);

  void SetCopy(bool copy);

 signals:
  void FileCopied(int);

 public slots:
  void accept();
  void FileCopied_(int);

 protected:
  void showEvent(QShowEvent*);
  void resizeEvent(QResizeEvent*);

 private slots:
  void Reset();

  void InsertTag(const QString& tag);
  void UpdatePreviews();

  void OrganiseFinished(const QStringList& files_with_errors);

 private:
  SongList LoadSongsBlocking(const QStringList& filenames);
  void SetLoadingSongs(bool loading);

  static Organise::NewSongInfoList ComputeNewSongsFilenames(
      const SongList& songs, const OrganiseFormat& format);

  Ui_OrganiseDialog* ui_;
  TaskManager* task_manager_;

  OrganiseFormat format_;

  QFuture<SongList> songs_future_;
  SongList songs_;
  Organise::NewSongInfoList new_songs_info_;
  quint64 total_size_;

  std::unique_ptr<OrganiseErrorDialog> error_dialog_;

  bool resized_by_user_;

  FRIEND_TEST(OrganiseDialogTest, ComputeNewSongsFilenamesTest);
};

#endif  // ORGANISEDIALOG_H
