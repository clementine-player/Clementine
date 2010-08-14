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

#include "wpdlister.h"

// Vista, for the property system
#define WINVER 0x0501

#include <objbase.h>

// I have no idea where these are meant to be defined...
typedef struct _tagpropertykey {
  GUID fmtid;
  DWORD pid;
} PROPERTYKEY;
typedef const PROPERTYKEY& REFPROPERTYKEY;

#include <PortableDeviceApi.h>

#include <QStringList>
#include <QtDebug>

WpdLister::WpdLister() {
}

void WpdLister::Init() {
  qDebug() << "Creating IPortableDeviceManager";

  qDebug() << "CoInitialize says" << CoInitialize(0);

  IPortableDeviceManager* manager = NULL;
  HRESULT ret = CoCreateInstance(
      CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER,
      IID_IPortableDeviceManager, (void**)&manager);
  qDebug() << "Created" << ret << manager;

  CoUninitialize();
}

QStringList WpdLister::DeviceUniqueIDs() {
  return QStringList();
}

QStringList WpdLister::DeviceIcons(const QString& id) {
  return QStringList();
}

QString WpdLister::DeviceManufacturer(const QString& id) {
  return QString();
}

QString WpdLister::DeviceModel(const QString& id) {
  return QString();
}

quint64 WpdLister::DeviceCapacity(const QString& id) {
  return 0;
}

quint64 WpdLister::DeviceFreeSpace(const QString& id) {
  return 0;
}

QVariantMap WpdLister::DeviceHardwareInfo(const QString& id) {
  return QVariantMap();
}

QString WpdLister::MakeFriendlyName(const QString& id) {
  return QString();
}

QList<QUrl> WpdLister::MakeDeviceUrls(const QString& id) {
  return QList<QUrl>();
}

void WpdLister::UnmountDevice(const QString& id) {
}

void WpdLister::UpdateDeviceFreeSpace(const QString& id) {
}
