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

#include "imobiledeviceconnection.h"

#include <plist/plist.h>

#include <QCoreApplication>
#include <QtDebug>

iMobileDeviceConnection::iMobileDeviceConnection(const QString& uuid)
    : device_(NULL), lockdown_(NULL), afc_(NULL), afc_port_(0) {
  idevice_error_t err = idevice_new(&device_, uuid.toUtf8().constData());
  if (err != IDEVICE_E_SUCCESS) {
    qWarning() << "idevice error:" << err;
    return;
  }

  const char* label = QCoreApplication::applicationName().toUtf8().constData();
  lockdownd_error_t lockdown_err =
      lockdownd_client_new_with_handshake(device_, &lockdown_, label);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    qWarning() << "lockdown error:" << lockdown_err;
    return;
  }

  lockdown_err = lockdownd_start_service(lockdown_, "com.apple.afc", &afc_port_);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    qWarning() << "lockdown error:" << lockdown_err;
    return;
  }

  afc_error_t afc_err = afc_client_new(device_, afc_port_, &afc_);
  if (afc_err != 0) {
    qWarning() << "afc error:" << afc_err;
    return;
  }
}

iMobileDeviceConnection::~iMobileDeviceConnection() {
  if (afc_) {
    afc_client_free(afc_);
  }
  if (lockdown_) {
    lockdownd_client_free(lockdown_);
  }
  if (device_) {
    idevice_free(device_);
  }
}

QString iMobileDeviceConnection::GetProperty(const QString& property) {
  plist_t node = NULL;
  lockdownd_get_value(lockdown_, NULL, property.toUtf8().constData(), &node);
  char* value = NULL;
  plist_get_string_val(node, &value);
  plist_free(node);

  QString ret = QString::fromUtf8(value);
  free(value);
  return ret;
}

quint64 iMobileDeviceConnection::GetInfoLongLong(const QString& key) {
  char* value = NULL;
  afc_error_t err = afc_get_device_info_key(afc_, key.toUtf8().constData(), &value);
  if (err != AFC_E_SUCCESS || !value) {
    return 0;
  }
  QString num = QString::fromAscii(value);
  quint64 ret = num.toULongLong();
  free(value);
  return ret;
}

QStringList iMobileDeviceConnection::ReadDirectory(const QString& path,
                                                   QDir::Filters filters) {
  char** list = NULL;
  afc_error_t err = afc_read_directory(afc_, path.toUtf8().constData(), &list);
  if (err != AFC_E_SUCCESS || !list) {
    return QStringList();
  }

  QStringList ret;
  for (char** p = list ; *p != NULL ; ++p) {
    QString filename = QString::fromUtf8(*p);
    free(*p);

    if (filters == QDir::NoFilter)
      ret << filename;
    else {
      if (filters & QDir::NoDotAndDotDot && (filename == "." || filename == ".."))
        continue;
      if (!(filters & QDir::Hidden) && filename.startsWith("."))
        continue;

      QString filetype = GetFileInfo(path + "/" + filename, "st_ifmt");
      if ((filetype == "S_IFREG" && (filters & QDir::Files)) ||
          (filetype == "S_IFDIR" && (filters & QDir::Dirs)) ||
          (filetype == "S_IFLNK" && (!(filters & QDir::NoSymLinks))))
        ret << filename;
    }
  }
  free(list);

  return ret;
}

QString iMobileDeviceConnection::GetFileInfo(const QString& path, const QString& key) {
  QString ret;
  char** infolist = NULL;
  afc_error_t err = afc_get_file_info(afc_, path.toUtf8().constData(), &infolist);
  if (err != AFC_E_SUCCESS || !infolist) {
    return ret;
  }

  QString last_key;
  for (char** p = infolist ; *p != NULL ; ++p) {
    if (last_key.isNull()) {
      last_key = QString::fromUtf8(*p);
    } else {
      if (last_key == key)
        ret = QString::fromUtf8(*p);
      last_key = QString();
    }
    free(*p);
  }
  free(infolist);
  return ret;
}
