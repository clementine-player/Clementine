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

#ifndef ORGANISE_H
#define ORGANISE_H

#include <QObject>

#include <boost/shared_ptr.hpp>

#include "organiseformat.h"

class MusicStorage;
class TaskManager;

class Organise : public QObject {
  Q_OBJECT

public:
  Organise(TaskManager* task_manager,
           boost::shared_ptr<MusicStorage> destination,
           const OrganiseFormat& format, bool copy, bool overwrite,
           const QStringList& files, bool eject_after);

  static const int kBatchSize;

  void Start();

signals:
  void Finished(const QStringList& files_with_errors);

private slots:
  void ProcessSomeFiles();
  void SetSongProgress(float progress);
  void UpdateProgress();

private:
  QThread* thread_;
  QThread* original_thread_;
  TaskManager* task_manager_;
  boost::shared_ptr<MusicStorage> destination_;

  const OrganiseFormat format_;
  const bool copy_;
  const bool overwrite_;
  QStringList files_;
  const bool eject_after_;

  bool started_;

  int task_id_;
  int progress_;
  int song_progress_;

  QStringList files_with_errors_;
};

#endif // ORGANISE_H
