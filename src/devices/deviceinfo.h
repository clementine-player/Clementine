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

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <memory>

#include <QIcon>
#include <QList>
#include <QString>
#include <QVariant>

#include "core/musicstorage.h"
#include "core/simpletreeitem.h"
#include "core/simpletreemodel.h"
#include "core/song.h"
#include "devicedatabasebackend.h"
#include "library/librarymodel.h"

class ConnectedDevice;
class DeviceLister;

// Devices can be in three different states:
//  1) Remembered in the database but not physically connected at the moment.
//     database_id valid, lister null, device null
//  2) Physically connected but the user hasn't "connected" it to Clementine
//     yet.
//     database_id == -1, lister valid, device null
//  3) Physically connected and connected to Clementine
//     database_id valid, lister valid, device valid
// Devices in all states will have a unique_id.
class DeviceInfo : public SimpleTreeItem<DeviceInfo> {
 public:
  enum Type {
    Type_Root,
    Type_Device,
  };

  DeviceInfo(SimpleTreeModel<DeviceInfo>* model)
      : SimpleTreeItem<DeviceInfo>(Type_Root, model),
        database_id_(-1),
        size_(0),
        transcode_mode_(MusicStorage::Transcode_Unsupported),
        transcode_format_(Song::Type_Unknown),
        task_percentage_(-1) {}

  DeviceInfo(Type type, DeviceInfo* parent = nullptr)
      : SimpleTreeItem<DeviceInfo>(type, parent),
        database_id_(-1),
        size_(0),
        transcode_mode_(MusicStorage::Transcode_Unsupported),
        transcode_format_(Song::Type_Unknown),
        task_percentage_(-1) {}

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

#endif  // DEVICEINFO_H
