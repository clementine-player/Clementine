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

#include <QObject>

class ConnectedDevice;
class DeviceLister;
class TaskManager;

class DeviceManager : public QObject {
  Q_OBJECT

public:
  DeviceManager(TaskManager* task_manager, QObject* parent = 0);
  ~DeviceManager();

  TaskManager* task_manager() const { return task_manager_; }

private slots:
  void DeviceAdded(const QString& id);
  void DeviceRemoved(const QString& id);
  void DeviceChanged(const QString& id);

private:
  void AddLister(DeviceLister* lister);

private:
  TaskManager* task_manager_;

  QList<DeviceLister*> listers_;
  QList<ConnectedDevice*> devices_;
};

#endif // DEVICEMANAGER_H
