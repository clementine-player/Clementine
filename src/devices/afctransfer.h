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

#ifndef AFCTRANSFER_H
#define AFCTRANSFER_H

#include <QObject>
#include <QStringList>

#include <boost/shared_ptr.hpp>

class ConnectedDevice;
class iMobileDeviceConnection;
class TaskManager;

class QIODevice;

class AfcTransfer : public QObject {
  Q_OBJECT

public:
  AfcTransfer(const QString& uuid, const QString& local_destination,
              TaskManager* task_manager, boost::shared_ptr<ConnectedDevice> device);
  ~AfcTransfer();

  bool CopyToDevice(iMobileDeviceConnection* connection);

public slots:
  void CopyFromDevice();

signals:
  void TaskStarted(int task_id);
  void CopyFinished(bool success);

private:
  bool CopyDirFromDevice(iMobileDeviceConnection* c, const QString& path);
  bool CopyDirToDevice(iMobileDeviceConnection* c, const QString& path);
  bool CopyFileFromDevice(iMobileDeviceConnection* c, const QString& path);
  bool CopyFileToDevice(iMobileDeviceConnection* c, const QString& path);

private:
  boost::shared_ptr<ConnectedDevice> device_;
  QThread* original_thread_;

  TaskManager* task_manager_;
  QString uuid_;
  QString local_destination_;

  QStringList important_directories_;
};

#endif // AFCTRANSFER_H
