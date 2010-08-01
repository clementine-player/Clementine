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

#ifndef GPODDEVICE_H
#define GPODDEVICE_H

#include "connecteddevice.h"
#include "core/musicstorage.h"

#include <QMutex>
#include <QWaitCondition>

#include <gpod/itdb.h>

class GPodLoader;

class GPodDevice : public ConnectedDevice, public virtual MusicStorage {
  Q_OBJECT

public:
  Q_INVOKABLE GPodDevice(
      const QUrl& url, DeviceLister* lister,
      const QString& unique_id, DeviceManager* manager,
      int database_id, bool first_time);
  ~GPodDevice();

  static QStringList url_schemes() { return QStringList() << "ipod"; }

  void StartCopy();
  bool CopyToStorage(const QString &source, const QString &destination,
                     const Song &metadata, bool overwrite, bool remove_original);
  void FinishCopy();

  void StartDelete();
  bool DeleteFromStorage(const Song& metadata);
  void FinishDelete();

private slots:
  void LoadFinished(Itdb_iTunesDB* db);

private:
  QThread* loader_thread_;
  GPodLoader* loader_;

  QWaitCondition db_wait_cond_;
  QMutex db_mutex_;
  Itdb_iTunesDB* db_;

  QMutex db_busy_;
  SongList songs_to_add_;
  SongList songs_to_remove_;
};

#endif // GPODDEVICE_H
