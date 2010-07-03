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

#include "connecteddevice.h"
#include "devicemanager.h"
#include "devicekitlister.h"
#include "core/utilities.h"
#include "ui/iconloader.h"

#include <QIcon>


DeviceManager::DeviceInfo::DeviceInfo()
  : database_id_(-1),
    lister_(NULL)
{
}

DeviceManager::DeviceManager(BackgroundThread<Database>* database,
                             TaskManager* task_manager, QObject *parent)
  : QAbstractListModel(parent),
    database_(database),
    task_manager_(task_manager)
{
#ifdef Q_WS_X11
  AddLister(new DeviceKitLister);
#endif
}

DeviceManager::~DeviceManager() {
  qDeleteAll(listers_);
}

int DeviceManager::rowCount(const QModelIndex&) const {
  return devices_.count();
}

QVariant DeviceManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.column() != 0)
    return QVariant();

  const DeviceInfo& info = devices_[index.row()];

  switch (role) {
  case Qt::DisplayRole: {
    QString text = info.friendly_name_.isEmpty() ? info.unique_id_ : info.friendly_name_;
    if (info.size_)
      text = text + QString(" (%1)").arg(Utilities::PrettySize(info.size_));
    return text;
  }

  case Qt::DecorationRole: {
    bool connected = info.lister_;
    return info.icon_.pixmap(22, connected ? QIcon::Normal : QIcon::Disabled);
  }

  default:
    return QVariant();
  }
}

void DeviceManager::AddLister(DeviceLister *lister) {
  listers_ << lister;
  connect(lister, SIGNAL(DeviceAdded(QString)), SLOT(PhysicalDeviceAdded(QString)));
  connect(lister, SIGNAL(DeviceRemoved(QString)), SLOT(PhysicalDeviceRemoved(QString)));
  connect(lister, SIGNAL(DeviceChanged(QString)), SLOT(PhysicalDeviceChanged(QString)));

  lister->Start();
}

int DeviceManager::FindDeviceById(const QString &id) const {
  for (int i=0 ; i<devices_.count() ; ++i) {
    if (devices_[i].unique_id_ == id)
      return i;
  }
  return -1;
}

void DeviceManager::PhysicalDeviceAdded(const QString &id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());

  qDebug() << "Device added:" << id;
  for (int i=0 ; i<DeviceKitLister::LastField ; ++i) {
    qDebug() << i << lister->DeviceInfo(id, i);
  }

  // Do we have this device already?
  int i = FindDeviceById(id);
  if (i == -1) {
    DeviceInfo info;
    info.lister_ = lister;
    info.unique_id_ = id;
    info.friendly_name_ = lister->DeviceInfo(id, DeviceLister::Field_FriendlyName).toString();
    info.size_ = lister->DeviceInfo(id, DeviceLister::Field_Capacity).toLongLong();

    // Try to load the icon with that exact name first
    QString icon_name = lister->DeviceInfo(id, DeviceLister::Field_Icon).toString();
    info.icon_ = IconLoader::Load(icon_name);

    // If that failed than try to guess if it's a phone or ipod.  Fall back on
    // a usb memory stick icon.
    if (info.icon_.isNull()) {
      if (icon_name.contains("phone"))
        info.icon_ = IconLoader::Load("phone");
      else if (icon_name.contains("ipod") || icon_name.contains("apple"))
        info.icon_ = IconLoader::Load("multimedia-player-ipod-standard-monochrome");
      else
        info.icon_ = IconLoader::Load("drive-removable-media-usb-pendrive");
    }

    beginInsertRows(QModelIndex(), devices_.count(), devices_.count());
    devices_ << info;
    endInsertRows();
  } else {
    DeviceInfo& info = devices_[i];

    // TODO: Make a ConnectedDevice
  }
}

void DeviceManager::PhysicalDeviceRemoved(const QString &id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());
  qDebug() << "Device removed:" << id;

  int i = FindDeviceById(id);
  if (i == -1) {
    // Shouldn't happen
    return;
  }

  DeviceInfo& info = devices_[i];

  if (info.database_id_ != -1) {
    // Keep the structure around, but just "disconnect" it
    info.lister_ = NULL;
    info.device_.reset();

    emit dataChanged(index(i, 0), index(i, 0));
  } else {
    // Remove the item from the model
    beginRemoveRows(QModelIndex(), i, i);
    devices_.removeAt(i);
    endRemoveRows();
  }
}

void DeviceManager::PhysicalDeviceChanged(const QString &id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());

  int i = FindDeviceById(id);
  if (i == -1) {
    // Shouldn't happen
    return;
  }

  // TODO
}
