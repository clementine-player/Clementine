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

#include "imobiledeviceconnection.h"
#include "core/logging.h"

#include <plist/plist.h>

#include <QCoreApplication>
#include <QUrl>
#include <QtDebug>

iMobileDeviceConnection::iMobileDeviceConnection(const QString& uuid)
    : device_(NULL), afc_(NULL), afc_port_(0) {
  idevice_error_t err = idevice_new(&device_, uuid.toUtf8().constData());
  if (err != IDEVICE_E_SUCCESS) {
    qLog(Warning) << "idevice error:" << err;
    return;
  }

  lockdownd_client_t lockdown;

  QByteArray label_ascii = QCoreApplication::applicationName().toAscii();
  const char* label = label_ascii.constData();
  lockdownd_error_t lockdown_err =
      lockdownd_client_new_with_handshake(device_, &lockdown, label);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    qLog(Warning) << "lockdown error:" << lockdown_err;
    return;
  }

  lockdown_err = lockdownd_start_service(lockdown, "com.apple.afc", &afc_port_);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    qLog(Warning) << "lockdown error:" << lockdown_err;
    lockdownd_client_free(lockdown);
    return;
  }

  afc_error_t afc_err = afc_client_new(device_, afc_port_, &afc_);
  if (afc_err != 0) {
    qLog(Warning) << "afc error:" << afc_err;
    lockdownd_client_free(lockdown);
    return;
  }

  lockdownd_client_free(lockdown);
}

iMobileDeviceConnection::~iMobileDeviceConnection() {
  if (afc_) {
    afc_client_free(afc_);
  }
  if (device_) {
    idevice_free(device_);
  }
}

template <typename T, typename F>
T GetPListValue(plist_t node, F f) {
  T ret;
  f(node, &ret);
  return ret;
}

QVariant iMobileDeviceConnection::GetProperty(const QString& property, const QString& domain) {
  lockdownd_client_t lockdown;
  QByteArray label_ascii = QCoreApplication::applicationName().toAscii();
  const char* label = label_ascii.constData();

  lockdownd_error_t lockdown_err =
      lockdownd_client_new_with_handshake(device_, &lockdown, label);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    qLog(Warning) << "lockdown error:" << lockdown_err;
    return QVariant();
  }

  plist_t node = NULL;
  QByteArray domain_ascii = domain.toAscii();
  const char* d = domain_ascii.isEmpty() ? NULL : domain_ascii.constData();
  //const char* d = domain.isEmpty() ? NULL : "com.apple.disk_usage";
  lockdownd_get_value(lockdown, d, property.toAscii().constData(), &node);
  lockdownd_client_free(lockdown);

  if (!node) {
    qLog(Warning) << "get_value failed" << property << domain;
    return QVariant();
  }

  switch (plist_get_node_type(node)) {
    case PLIST_BOOLEAN:
      return bool(GetPListValue<quint8>(node, plist_get_bool_val));

    case PLIST_UINT:
      return QVariant::fromValue(GetPListValue<uint64_t>(node, plist_get_uint_val));

    case PLIST_STRING: {
      char* data = GetPListValue<char*>(node, plist_get_string_val);
      QString ret = QString::fromUtf8(data);
      free(data);
      return ret;
    }

    default:
      qLog(Warning) << "Unhandled PList type";
      return QVariant();
  }
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

bool iMobileDeviceConnection::MkDir(const QString& path) {
  afc_error_t err = afc_make_directory(afc_, path.toUtf8().constData());
  return err == AFC_E_SUCCESS;
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

bool iMobileDeviceConnection::Exists(const QString& path) {
  return !GetFileInfo(path, "st_ifmt").isNull();
}

QString iMobileDeviceConnection::GetUnusedFilename(
    Itdb_iTunesDB* itdb, const Song& metadata) {
  // This function does the same as itdb_cp_get_dest_filename, except it
  // accesses the device's filesystem through imobiledevice.

  // Get the total number of F.. directories
  int total_musicdirs = 0;
  for ( ; ; ++total_musicdirs) {
    QString dir;
    dir.sprintf("/iTunes_Control/Music/F%02d", total_musicdirs);

    if (!Exists(dir))
      break;
  }

  if (total_musicdirs <= 0) {
    qLog(Warning) << "No 'F..'' directories found on iPod";
    return QString();
  }

  // Pick one at random
  const int dir_num = qrand() % total_musicdirs;
  QString dir;
  dir.sprintf("/iTunes_Control/Music/F%02d", dir_num);

  if (!Exists(dir)) {
    qLog(Warning) << "Music directory doesn't exist:" << dir;
    return QString();
  }

  // Use the same file extension as the original file, default to mp3.
  QString extension = metadata.url().path().section('.', -1, -1).toLower();
  if (extension.isEmpty())
    extension = "mp3";

  // Loop until we find an unused filename.
  // Use the same naming convention as libgpod, which is
  // "libgpod" + 6-digit random number
  static const int kRandMax = 999999;
  QString filename;
  forever {
    filename.sprintf("libgpod%06d", qrand() % kRandMax);
    filename += "." + extension;

    if (!Exists(dir + "/" + filename))
      break;
  }

  return dir + "/" + filename;
}
