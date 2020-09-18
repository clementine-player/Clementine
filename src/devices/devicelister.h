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

#ifndef DEVICELISTER_H
#define DEVICELISTER_H

#include <QAbstractItemModel>
#include <QUrl>

class ConnectedDevice;
class DeviceManager;

class DeviceLister : public QObject {
  Q_OBJECT

 public:
  DeviceLister();
  virtual ~DeviceLister();

  // Tries to start the thread and initialise the engine.  This object will be
  // moved to the new thread.
  void Start();

  // If two listers know about the same device, then the metadata will get
  // taken from the one with the highest priority.
  virtual int priority() const { return 100; }

  // Query information about the devices that are available.  Must be
  // thread-safe.
  virtual QStringList DeviceUniqueIDs() = 0;
  virtual QVariantList DeviceIcons(const QString& id) = 0;
  virtual QString DeviceManufacturer(const QString& id) = 0;
  virtual QString DeviceModel(const QString& id) = 0;
  virtual quint64 DeviceCapacity(const QString& id) = 0;
  virtual quint64 DeviceFreeSpace(const QString& id) = 0;
  virtual QVariantMap DeviceHardwareInfo(const QString& id) = 0;
  virtual bool DeviceNeedsMount(const QString& id) { return false; }

  // When connecting to a device for the first time, do we want an user's
  // confirmation for scanning it? (by default yes)
  virtual bool AskForScan(const QString&) const { return true; }

  virtual QString MakeFriendlyName(const QString& id) = 0;
  virtual QList<QUrl> MakeDeviceUrls(const QString& id) = 0;

  // Ensure the device is mounted.  This should run asynchronously and emit
  // DeviceMounted when it's done.
  virtual int MountDevice(const QString& id);

  // Do whatever needs to be done to safely remove the device.
  virtual void UnmountDevice(const QString& id) = 0;

 public slots:
  virtual void UpdateDeviceFreeSpace(const QString& id) = 0;
  virtual void ShutDown() {}

 signals:
  void DeviceAdded(const QString& id);
  void DeviceRemoved(const QString& id);
  void DeviceChanged(const QString& id);
  void DeviceMounted(const QString& id, int request_id, bool success);

 protected:
  virtual void Init() = 0;
  QUrl MakeUrlFromLocalPath(const QString& path) const;
  bool IsIpod(const QString& path) const;

  QStringList GuessIconForPath(const QString& path);
  QStringList GuessIconForModel(const QString& vendor, const QString& model);

 protected:
  QThread* thread_;
  int next_mount_request_id_;

 private slots:
  void ThreadStarted();
};

#endif  // DEVICELISTER_H
