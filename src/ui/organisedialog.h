/* This file is part of Clementine.

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

#include <QDialog>
#include <QMap>
#include <QUrl>

#include "core/organiseformat.h"
#include "core/song.h"

#include <boost/shared_ptr.hpp>

class LibraryWatcher;
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

  void SetUrls(const QList<QUrl>& urls, quint64 total_size = 0);
  void SetFilenames(const QStringList& filenames, quint64 total_size = 0);
  void SetCopy(bool copy);

public slots:
  void accept();

protected:
  void showEvent(QShowEvent *);

private slots:
  void Reset();

  void InsertTag(const QString& tag);
  void LoadPreviewSongs(const QString& filename);
  void UpdatePreviews();

private:
  Ui_OrganiseDialog* ui_;
  TaskManager* task_manager_;

  OrganiseFormat format_;

  QStringList filenames_;
  SongList preview_songs_;
  quint64 total_size_;
};

#endif // ORGANISEDIALOG_H
