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

#ifndef MTPDEVICE_H
#define MTPDEVICE_H

#include <memory>

#include <QMutex>
#include <QWaitCondition>

#include "connecteddevice.h"

struct LIBMTP_mtpdevice_struct;

class MtpConnection;
class MtpLoader;

class MtpDevice : public ConnectedDevice {
  Q_OBJECT

 public:
  Q_INVOKABLE MtpDevice(const QUrl& url, DeviceLister* lister,
                        const QString& unique_id, DeviceManager* manager,
                        Application* app, int database_id, bool first_time);
  ~MtpDevice();

  static QStringList url_schemes() {
    return QStringList() << "mtp"
                         << "gphoto2";
  }

  void Init();
  void ConnectAsync();

  bool GetSupportedFiletypes(QList<Song::FileType>* ret);
  int GetFreeSpace();
  int GetCapacity();

  bool StartCopy(QList<Song::FileType>* supported_types);
  bool CopyToStorage(const CopyJob& job);
  void FinishCopy(bool success);

  void StartDelete();
  bool DeleteFromStorage(const DeleteJob& job);
  void FinishDelete(bool success);

 private slots:
  void LoadFinished(bool success);
  void LoaderError(const QString& message);

 private:
  bool GetSupportedFiletypes(QList<Song::FileType>* ret,
                             LIBMTP_mtpdevice_struct* device);
  int GetFreeSpace(LIBMTP_mtpdevice_struct* device);
  int GetCapacity(LIBMTP_mtpdevice_struct* device);

 private:
  static bool sInitialisedLibMTP;

  QThread* loader_thread_;
  MtpLoader* loader_;

  QMutex db_busy_;
  SongList songs_to_add_;
  SongList songs_to_remove_;

  std::unique_ptr<MtpConnection> connection_;
};

#endif  // MTPDEVICE_H
