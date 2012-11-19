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

#include "wmdmthread.h"
#include "core/logging.h"
#include "core/utilities.h"

#include <mswmdm.h>

#include <boost/scoped_array.hpp>

#include <QCoreApplication>
#include <QLibrary>
#include <QMutexLocker>
#include <QtDebug>

BYTE abPVK[] = {0x00};
BYTE abCert[] = {0x00};

bool WmdmThread::sIsLoaded = false;

decltype(&CSecureChannelClient_New) WmdmThread::_CSecureChannelClient_New;
decltype(&CSecureChannelClient_Free) WmdmThread::_CSecureChannelClient_Free;
decltype(&CSecureChannelClient_SetCertificate) WmdmThread::_CSecureChannelClient_SetCertificate;
decltype(&CSecureChannelClient_SetInterface) WmdmThread::_CSecureChannelClient_SetInterface;  
decltype(&CSecureChannelClient_Authenticate) WmdmThread::_CSecureChannelClient_Authenticate;


WmdmThread::WmdmThread()
  : device_manager_(NULL),
    sac_(NULL)
{
  if (!sIsLoaded) {
    return;
  }
  // Initialise COM
  CoInitialize(0);

  // Authenticate with WMDM
  IComponentAuthenticate* auth;
  if (CoCreateInstance(CLSID_MediaDevMgr, NULL, CLSCTX_ALL,
                       IID_IComponentAuthenticate, (void**) &auth)) {
    qLog(Warning) << "Error creating the IComponentAuthenticate interface";
    return;
  }

  sac_ = _CSecureChannelClient_New();
  if (_CSecureChannelClient_SetCertificate(
      sac_, SAC_CERT_V1, abCert, sizeof(abCert), abPVK, sizeof(abPVK))) {
    qLog(Warning) << "Error setting SAC certificate";
    return;
  }

  _CSecureChannelClient_SetInterface(sac_, auth);
  if (_CSecureChannelClient_Authenticate(sac_, SAC_PROTOCOL_V1)) {
    qLog(Warning) << "Error authenticating with SAC";
    return;
  }

  // Create the device manager
  if (auth->QueryInterface(IID_IWMDeviceManager2, (void**)&device_manager_)) {
    qLog(Warning) << "Error creating WMDM device manager";
    return;
  }
}

WmdmThread::~WmdmThread() {
  if (device_manager_) {
    // Release the device manager
    device_manager_->Release();
  }

  if (sac_) {
    // SAC
    _CSecureChannelClient_Free(sac_);
  }

  // Uninitialise COM
  CoUninitialize();
}

namespace {

template <typename T>
T Resolve(QLibrary* library, const char* name) {
  return reinterpret_cast<T>(library->resolve(name));
}

}  // namespace

bool WmdmThread::StaticInit() {
  if (!sIsLoaded) {
    QLibrary library(QCoreApplication::applicationDirPath() + "/sac_shim.dll");
    if (!library.load()) {
      return false;
    }

    _CSecureChannelClient_New = Resolve<decltype(_CSecureChannelClient_New)>(
        &library, "CSecureChannelClient_New");
    _CSecureChannelClient_Free = Resolve<decltype(_CSecureChannelClient_Free)>(
        &library, "CSecureChannelClient_Free");
    _CSecureChannelClient_SetCertificate = Resolve<decltype(_CSecureChannelClient_SetCertificate)>(
        &library, "CSecureChannelClient_SetCertificate");
    _CSecureChannelClient_SetInterface = Resolve<decltype(_CSecureChannelClient_SetInterface)>(
        &library, "CSecureChannelClient_SetInterface");
    if (_CSecureChannelClient_New &&
        _CSecureChannelClient_Free &&
        _CSecureChannelClient_SetCertificate &&
        _CSecureChannelClient_SetInterface) {
      sIsLoaded = true;
      return true;
    }
  }
  return false;
}

IWMDMDevice* WmdmThread::GetDeviceByCanonicalName(const QString& device_name) {
  ScopedWCharArray name(device_name);

  IWMDMDevice* device = NULL;
  if (device_manager_->GetDeviceFromCanonicalName(name, &device)) {
    qLog(Warning) << "Error in GetDeviceFromCanonicalName for" << device_name;
    return NULL;
  }

  return device;
}

IWMDMStorage* WmdmThread::GetRootStorage(const QString& device_name) {
  IWMDMDevice* device = GetDeviceByCanonicalName(device_name);

  IWMDMEnumStorage* storage_it = NULL;
  device->EnumStorage(&storage_it);

  ULONG storage_fetched = 0;
  IWMDMStorage* storage = NULL;
  storage_it->Next(1, &storage, &storage_fetched);

  storage_it->Release();
  device->Release();

  return storage;
}
