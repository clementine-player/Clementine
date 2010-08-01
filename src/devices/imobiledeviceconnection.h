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

#ifndef IMOBILEDEVICECONNECTION_H
#define IMOBILEDEVICECONNECTION_H

#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#include <QDir>
#include <QStringList>

class iMobileDeviceConnection {
public:
  iMobileDeviceConnection(const QString& uuid);
  ~iMobileDeviceConnection();

  afc_client_t afc() { return afc_; }

  QString GetProperty(const QString& property);
  quint64 GetInfoLongLong(const QString& key);
  QStringList ReadDirectory(const QString& path, QDir::Filters filters = QDir::NoFilter);

private:
  Q_DISABLE_COPY(iMobileDeviceConnection);

  QString GetFileInfo(const QString& path, const QString& key);

  idevice_t device_;
  lockdownd_client_t lockdown_;
  afc_client_t afc_;

  uint16_t afc_port_;
};

#endif // IMOBILEDEVICECONNECTION_H
