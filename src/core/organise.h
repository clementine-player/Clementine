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

#include "organiseformat.h"

class TaskManager;

class Organise : public QObject {
  Q_OBJECT

public:
  Organise(TaskManager* task_manager, const QString& destination,
           const OrganiseFormat& format, bool copy, bool overwrite,
           const QStringList& files);

  static const int kBatchSize;

  void Start();

private slots:
  void ProcessSomeFiles();

private:
  QThread* thread_;
  QThread* original_thread_;
  TaskManager* task_manager_;

  const QString destination_;
  const OrganiseFormat format_;
  const bool copy_;
  const bool overwrite_;
  QStringList files_;

  int task_id_;
  int progress_;
};

#endif // ORGANISE_H
