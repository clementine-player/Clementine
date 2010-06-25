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

#ifndef DEVICEENGINE_H
#define DEVICEENGINE_H

#include <QAbstractItemModel>

class DeviceEngine : public QAbstractItemModel {
  Q_OBJECT

public:
  DeviceEngine(QObject* parent = 0);

  enum Column {
    Column_UniqueID = 0,
    Column_FriendlyName,
    Column_Manufacturer,
    Column_Model,
    Column_Capacity,
    Column_FreeSpace,

    LastDeviceEngineColumn
  };

  virtual bool Init() = 0;
};

#endif // DEVICEENGINE_H
