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

#include <boost/scoped_ptr.hpp>

class MtpConnection;
class MtpLoader;

class MtpDevice : public ConnectedDevice {
  Q_OBJECT

public:
  Q_INVOKABLE MtpDevice(const QUrl& url, DeviceLister* lister,
                        const QString& unique_id, DeviceManager* manager,
                        int database_id, bool first_time);
  ~MtpDevice();

  static QStringList url_schemes() { return QStringList() << "mtp" << "gphoto2"; }

  void Init();

  void StartCopy();
  bool CopyToStorage(const QString& source, const QString& destination,
                     const Song& metadata, bool overwrite, bool remove_original);
  void FinishCopy(bool success);

  void StartDelete();
  bool DeleteFromStorage(const Song& metadata);
  void FinishDelete(bool success);

private slots:
  void LoadFinished();

private:
  static bool sInitialisedLibMTP;

  QThread* loader_thread_;
  MtpLoader* loader_;

  QMutex db_busy_;
  SongList songs_to_add_;
  SongList songs_to_remove_;

  boost::scoped_ptr<MtpConnection> connection_;
};

#endif // MTPDEVICE_H
