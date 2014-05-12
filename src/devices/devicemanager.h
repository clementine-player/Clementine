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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "devicedatabasebackend.h"

#include <memory>

#include <QAbstractListModel>
#include <QIcon>
#include <QThreadPool>

#include "library/librarymodel.h"

class Application;
class ConnectedDevice;
class Database;
class DeviceLister;
class DeviceStateFilterModel;
class TaskManager;

class DeviceManager : public QAbstractListModel {
  Q_OBJECT

 public:
  DeviceManager(Application* app, QObject* parent = nullptr);
  ~DeviceManager();

  enum Role {
    Role_State = LibraryModel::LastRole,
    Role_UniqueId,
    Role_FriendlyName,
    Role_Capacity,
    Role_FreeSpace,
    Role_IconName,
    Role_UpdatingPercentage,
    Role_MountPath,
    Role_TranscodeMode,
    Role_TranscodeFormat,
    Role_SongCount,
    LastRole,
  };

  enum State {
    State_Remembered,
    State_NotMounted,
    State_NotConnected,
    State_Connected,
  };

  static const int kDeviceIconSize;
  static const int kDeviceIconOverlaySize;

  DeviceStateFilterModel* connected_devices_model() const {
    return connected_devices_model_;
  }

  // Get info about devices
  int GetDatabaseId(int row) const;
  DeviceLister* GetLister(int row) const;
  std::shared_ptr<ConnectedDevice> GetConnectedDevice(int row) const;

  int FindDeviceById(const QString& id) const;
  int FindDeviceByUrl(const QList<QUrl>& url) const;

  // Actions on devices
  std::shared_ptr<ConnectedDevice> Connect(int row);
  void Disconnect(int row);
  void Forget(int row);
  void UnmountAsync(int row);

  void SetDeviceOptions(int row, const QString& friendly_name,
                        const QString& icon_name,
                        MusicStorage::TranscodeMode mode,
                        Song::FileType format);

  // QAbstractListModel
  int rowCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;

 public slots:
  void Unmount(int row);

signals:
  void DeviceConnected(int row);
  void DeviceDisconnected(int row);

 private slots:
  void PhysicalDeviceAdded(const QString& id);
  void PhysicalDeviceRemoved(const QString& id);
  void PhysicalDeviceChanged(const QString& id);
  void DeviceTaskStarted(int id);
  void TasksChanged();
  void DeviceSongCountUpdated(int count);
  void LoadAllDevices();

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

    // A device can be discovered in different ways (devicekit, gio, etc.)
    // Sometimes the same device is discovered more than once.  In this case
    // the device will have multiple "backends".
    struct Backend {
      Backend(DeviceLister* lister = nullptr, const QString& id = QString())
          : lister_(lister), unique_id_(id) {}

      DeviceLister* lister_;  // nullptr if not physically connected
      QString unique_id_;
    };

    // Serialising to the database
    void InitFromDb(const DeviceDatabaseBackend::Device& dev);
    DeviceDatabaseBackend::Device SaveToDb() const;

    // Tries to load a good icon for the device.  Sets icon_name_ and icon_.
    void LoadIcon(const QVariantList& icons, const QString& name_hint);

    // Gets the best backend available (the one with the highest priority)
    const Backend* BestBackend() const;

    int database_id_;  // -1 if not remembered in the database
    std::shared_ptr<ConnectedDevice>
        device_;  // nullptr if not connected to clementine
    QList<Backend> backends_;

    QString friendly_name_;
    quint64 size_;

    QString icon_name_;
    QIcon icon_;

    MusicStorage::TranscodeMode transcode_mode_;
    Song::FileType transcode_format_;

    int task_percentage_;
  };

  void AddLister(DeviceLister* lister);
  template <typename T>
  void AddDeviceClass();

  DeviceDatabaseBackend::Device InfoToDatabaseDevice(const DeviceInfo& info)
      const;

 private:
  Application* app_;
  DeviceDatabaseBackend* backend_;

  DeviceStateFilterModel* connected_devices_model_;

  QIcon not_connected_overlay_;

  QList<DeviceLister*> listers_;
  QList<DeviceInfo> devices_;

  QMultiMap<QString, QMetaObject> device_classes_;

  // Map of task ID to device index
  QMap<int, QPersistentModelIndex> active_tasks_;

  QThreadPool thread_pool_;
};

template <typename T>
void DeviceManager::AddDeviceClass() {
  QStringList schemes = T::url_schemes();
  QMetaObject obj = T::staticMetaObject;

  for (const QString& scheme : schemes) {
    device_classes_.insert(scheme, obj);
  }
}

#endif  // DEVICEMANAGER_H
