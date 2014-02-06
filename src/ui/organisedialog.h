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
  OrganiseDialog(TaskManager* task_manager, QWidget* parent = 0);
  ~OrganiseDialog();

  static const int kNumberOfPreviews;
  static const char* kDefaultFormat;
  static const char* kSettingsGroup;

  QSize sizeHint() const;

  void SetDestinationModel(QAbstractItemModel* model, bool devices = false);

  int SetSongs(const SongList& songs);
  int SetUrls(const QList<QUrl>& urls, quint64 total_size = 0);
  int SetFilenames(const QStringList& filenames, quint64 total_size = 0);
  void SetCopy(bool copy);

public slots:
  void accept();

protected:
  void showEvent(QShowEvent*);
  void resizeEvent(QResizeEvent*);

private slots:
  void Reset();

  void InsertTag(const QString& tag);
  void LoadPreviewSongs(const QString& filename);
  void UpdatePreviews();

  void OrganiseFinished(const QStringList& files_with_errors);

private:
  Ui_OrganiseDialog* ui_;
  TaskManager* task_manager_;

  OrganiseFormat format_;

  QStringList filenames_;
  SongList preview_songs_;
  quint64 total_size_;

  std::unique_ptr<OrganiseErrorDialog> error_dialog_;

  bool resized_by_user_;
};

#endif // ORGANISEDIALOG_H
