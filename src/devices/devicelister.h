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

#ifndef DEVICELISTER_H
#define DEVICELISTER_H

#include <QAbstractItemModel>

#include <boost/shared_ptr.hpp>

class ConnectedDevice;
class DeviceManager;

class DeviceLister : public QObject {
  Q_OBJECT

public:
  DeviceLister();
  ~DeviceLister();

  enum Field {
    Field_UniqueID = 0,
    Field_FriendlyName,
    Field_Icon,
    Field_Manufacturer,
    Field_Model,
    Field_Capacity,
    Field_FreeSpace,

    LastField
  };

  // Tries to start the thread and initialise the engine.  This object will be
  // moved to the new thread.
  void Start();

  // Query information about the devices that are available.  Must be thread-safe.
  virtual QStringList DeviceUniqueIDs() = 0;
  virtual QVariant DeviceInfo(const QString& id, int field) = 0;

  // Create a new ConnectedDevice instance for the given device.  Must be
  // thread-safe.
  virtual boost::shared_ptr<ConnectedDevice> Connect(
      const QString& unique_id, DeviceManager* manager, int database_id,
      bool first_time) = 0;

signals:
  void DeviceAdded(const QString& id);
  void DeviceRemoved(const QString& id);
  void DeviceChanged(const QString& id);

protected:
  virtual void Init() = 0;

protected:
  QThread* thread_;

private slots:
  void ThreadStarted();
};
Q_DECLARE_METATYPE(DeviceLister*);

#endif // DEVICELISTER_H
