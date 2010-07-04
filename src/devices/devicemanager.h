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

#include "devicedatabasebackend.h"
#include "core/backgroundthread.h"
#include "library/librarymodel.h"

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

  enum Role {
    Role_State = LibraryModel::LastRole,
    Role_UniqueId,
    Role_FriendlyName,
    Role_Capacity,
    Role_IconName,
    Role_UpdatingPercentage,
  };

  enum State {
    State_Remembered,
    State_NotConnected,
    State_Connected,
  };

  static const int kDeviceIconSize;
  static const int kDeviceIconOverlaySize;

  BackgroundThread<Database>* database() const { return database_; }
  TaskManager* task_manager() const { return task_manager_; }

  // Get info about devices
  int GetDatabaseId(int row) const;
  DeviceLister* GetLister(int row) const;
  boost::shared_ptr<ConnectedDevice> GetConnectedDevice(int row) const;

  int FindDeviceById(const QString& id) const;

  // Actions on devices
  boost::shared_ptr<ConnectedDevice> Connect(int row);
  void Disconnect(int row);
  void Forget(int row);

  void SetDeviceIdentity(int row, const QString& friendly_name,
                         const QString& icon_name);

  // QAbstractListModel
  int rowCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;

signals:
  void DeviceDisconnected(int row);

private slots:
  void PhysicalDeviceAdded(const QString& id);
  void PhysicalDeviceRemoved(const QString& id);
  void PhysicalDeviceChanged(const QString& id);
  void DeviceTaskStarted(int id);
  void TasksChanged();

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

    void InitFromDb(const DeviceDatabaseBackend::Device& dev);
    DeviceDatabaseBackend::Device SaveToDb() const;

    void LoadIcon(const QString& filename);

    int database_id_; // -1 if not remembered in the database
    DeviceLister* lister_; // NULL if not physically connected
    boost::shared_ptr<ConnectedDevice> device_; // NULL if not connected to clementine

    QString unique_id_;
    QString friendly_name_;
    quint64 size_;

    QString icon_name_;
    QIcon icon_;

    int task_percentage_;
  };

  void AddLister(DeviceLister* lister);

  DeviceDatabaseBackend::Device InfoToDatabaseDevice(const DeviceInfo& info) const;

private:
  BackgroundThread<Database>* database_;
  DeviceDatabaseBackend* backend_;
  TaskManager* task_manager_;

  QIcon not_connected_overlay_;

  QList<DeviceLister*> listers_;
  QList<DeviceInfo> devices_;

  // Map of task ID to device index
  QMap<int, QPersistentModelIndex> active_tasks_;
};

#endif // DEVICEMANAGER_H
