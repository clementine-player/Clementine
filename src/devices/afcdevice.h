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

#ifndef AFCDEVICE_H
#define AFCDEVICE_H

#include "connecteddevice.h"

#include <QMutex>
#include <QWaitCondition>

#include <gpod/itdb.h>

class AfcTransfer;
class GPodLoader;

class AfcDevice : public ConnectedDevice {
  Q_OBJECT

public:
  Q_INVOKABLE AfcDevice(const QUrl& url, DeviceLister* lister,
                        const QString& unique_id, DeviceManager* manager,
                        int database_id, bool first_time);
  ~AfcDevice();

  static QStringList url_schemes() { return QStringList() << "afc"; }

  bool CopyToStorage(const QString &source, const QString &destination,
                     const Song &metadata, bool overwrite, bool remove_original);
  bool DeleteFromStorage(const Song &metadata);

private slots:
  void CopyFinished();
  void LoadFinished(Itdb_iTunesDB* db);

private:
  void RemoveRecursive(const QString& path);

private:
  QThread* loader_thread_;
  AfcTransfer* transfer_;
  GPodLoader* loader_;

  QString local_path_;

  QWaitCondition db_wait_cond_;
  QMutex db_mutex_;
  Itdb_iTunesDB* db_;
};

#endif // AFCDEVICE_H
