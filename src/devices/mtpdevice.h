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

#ifndef MTPDEVICE_H
#define MTPDEVICE_H

#include "connecteddevice.h"

#include <QMutex>
#include <QWaitCondition>

class MtpLoader;

class MtpDevice : public ConnectedDevice {
  Q_OBJECT

public:
  Q_INVOKABLE MtpDevice(const QUrl& url, DeviceLister* lister,
                        const QString& unique_id, DeviceManager* manager,
                        int database_id, bool first_time);

  static QStringList url_schemes() { return QStringList() << "mtp" << "gphoto2"; }

  void Init();

  bool CopyToStorage(const QString& source, const QString& destination,
                     const Song& metadata, bool overwrite, bool remove_original);
  bool DeleteFromStorage(const Song& metadata);

private slots:
  void LoadFinished();

private:
  static bool sInitialisedLibMTP;

  QThread* loader_thread_;
  MtpLoader* loader_;

  QWaitCondition db_wait_cond_;
  QMutex db_mutex_;
};

#endif // MTPDEVICE_H
