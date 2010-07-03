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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "core/backgroundthread.h"

#include <QAbstractListModel>
#include <QIcon>

#include <boost/shared_ptr.hpp>

class ConnectedDevice;
class Database;
class DeviceLister;
class TaskManager;

class DeviceManager : public QAbstractListModel {
  Q_OBJECT

public:
  DeviceManager(BackgroundThread<Database>* database, TaskManager* task_manager,
                QObject* parent = 0);
  ~DeviceManager();

  BackgroundThread<Database>* database() const { return database_; }
  TaskManager* task_manager() const { return task_manager_; }

  // QAbstractListModel
  int rowCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;

private slots:
  void PhysicalDeviceAdded(const QString& id);
  void PhysicalDeviceRemoved(const QString& id);
  void PhysicalDeviceChanged(const QString& id);

private:
  void AddLister(DeviceLister* lister);
  int FindDeviceById(const QString& id) const;

private:
  // Devices can be in three different states:
  //  1) Remembered in the database but not physically connected at the moment.
  //     database_id valid, lister null, device null
  //  2) Physically connected but the user hasn't "connected" it to Clementine
  //     yet.
  //     database_id == -1, lister valid, device null
  //  3) Physically connected and connected to Clementine
  //     database_id valid, lister valid, device valid
  // Devices in all states will have a unique_id.
  struct DeviceInfo {
    DeviceInfo();

    int database_id_; // -1 if not remembered in the database
    DeviceLister* lister_; // NULL if not physically connected
    boost::shared_ptr<ConnectedDevice> device_; // NULL if not connected to clementine

    QString unique_id_;
    QString friendly_name_;
    quint64 size_;
    QIcon icon_;
  };

  BackgroundThread<Database>* database_;
  TaskManager* task_manager_;

  QList<DeviceLister*> listers_;
  QList<DeviceInfo> devices_;
};

#endif // DEVICEMANAGER_H
