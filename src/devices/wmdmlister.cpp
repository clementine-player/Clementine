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

#include "wmdmlister.h"

#include <icomponentauthenticate.h>
#include <objbase.h>
#include <sac_shim.h>
#include <mswmdm.h>
#include <mswmdm_i.c>

#include <QStringList>
#include <QtDebug>

BYTE abPVK[] = {0x00};
BYTE abCert[] = {0x00};

WmdmLister::WmdmLister()
{
}

void WmdmLister::Init() {
  qDebug() << "Creating IPortableDeviceManager";

  qDebug() << "CoInitialize says" << CoInitialize(0);

  IComponentAuthenticate* auth;
  HRESULT result = CoCreateInstance(
      CLSID_MediaDevMgr, NULL, CLSCTX_ALL, IID_IComponentAuthenticate, (void **)&auth);
  qDebug() << "Auth" << result << auth;

  SacHandle sac = CSecureChannelClient_New();
  qDebug() << "SAC is" << sac;

  result = CSecureChannelClient_SetCertificate(sac,
      SAC_CERT_V1,
      abCert, sizeof(abCert),
      abPVK, sizeof(abPVK));
  qDebug() << "SetCertificate" << result << abCert << sizeof(abCert);

  CSecureChannelClient_SetInterface(sac, auth);

  DWORD* prot;
  DWORD prot_count;
  result = auth->SACGetProtocols(&prot, &prot_count);
  qDebug() << "SACGetProtocols" << result << prot_count;
  qDebug() << "Prot is" << prot;
  qDebug() << "Prot is" << prot[0];

  result = CSecureChannelClient_Authenticate(sac, prot[0]);
  qDebug() << "Authenticate" << result;

  CoTaskMemFree(prot);

  IWMDeviceManager* device_manager;
  result = auth->QueryInterface(IID_IWMDeviceManager, (void**)device_manager);
  qDebug() << "Manager" << result << device_manager;

  CoUninitialize();
}

QStringList WmdmLister::DeviceUniqueIDs() {
  return QStringList();
}

QStringList WmdmLister::DeviceIcons(const QString& id) {
  return QStringList();
}

QString WmdmLister::DeviceManufacturer(const QString& id) {
  return QString();
}

QString WmdmLister::DeviceModel(const QString& id) {
  return QString();
}

quint64 WmdmLister::DeviceCapacity(const QString& id) {
  return 0;
}

quint64 WmdmLister::DeviceFreeSpace(const QString& id) {
  return 0;
}

QVariantMap WmdmLister::DeviceHardwareInfo(const QString& id) {
  return QVariantMap();
}

QString WmdmLister::MakeFriendlyName(const QString& id) {
  return QString();
}

QList<QUrl> WmdmLister::MakeDeviceUrls(const QString& id) {
  return QList<QUrl>();
}

void WmdmLister::UnmountDevice(const QString& id) {
}

void WmdmLister::UpdateDeviceFreeSpace(const QString& id) {
}
