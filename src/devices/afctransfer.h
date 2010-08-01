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

#ifndef AFCTRANSFER_H
#define AFCTRANSFER_H

#include <QObject>

class iMobileDeviceConnection;
class TaskManager;

class AfcTransfer : public QObject {
  Q_OBJECT

public:
  AfcTransfer(const QString& uuid, const QString& local_destination,
              TaskManager* task_manager, QObject* parent = 0);

public slots:
  void CopyFromDevice();
  void CopyToDevice();

signals:
  void TaskStarted(int task_id);
  void CopyFinished();

private:
  void CopyDirFromDevice(iMobileDeviceConnection* c, const QString& path);
  void CopyFileFromDevice(iMobileDeviceConnection* c, const QString& path);

private:
  QThread* original_thread_;

  TaskManager* task_manager_;
  QString uuid_;
  QString local_destination_;
};

#endif // AFCTRANSFER_H
