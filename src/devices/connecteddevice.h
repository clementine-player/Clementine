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

#ifndef CONNECTEDDEVICE_H
#define CONNECTEDDEVICE_H

#include "core/musicstorage.h"

#include <QObject>
#include <QStringList>
#include <QUrl>

#include <boost/enable_shared_from_this.hpp>

class Database;
class DeviceLister;
class DeviceManager;
class LibraryBackend;
class LibraryModel;

class ConnectedDevice : public QObject, public virtual MusicStorage,
                        public boost::enable_shared_from_this<ConnectedDevice> {
  Q_OBJECT

public:
  ConnectedDevice(const QUrl& url, DeviceLister* lister,
                  const QString& unique_id, DeviceManager* manager,
                  int database_id, bool first_time);
  ~ConnectedDevice();

  virtual void Init() = 0;

  DeviceLister* lister() const { return lister_; }
  QString unique_id() const { return unique_id_; }
  LibraryModel* model() const { return model_; }
  QUrl url() const { return url_; }

  virtual void FinishCopy(bool success);
  virtual void FinishDelete(bool success);

  virtual void Eject();

signals:
  void TaskStarted(int id);
  void Error(const QString& message);

protected:
  void InitBackendDirectory(const QString& mount_point, bool first_time, bool rewrite_path = true);

protected:
  QUrl url_;
  bool first_time_;
  DeviceLister* lister_;
  QString unique_id_;
  int database_id_;
  DeviceManager* manager_;

  LibraryBackend* backend_;
  LibraryModel* model_;
};

#endif // CONNECTEDDEVICE_H
