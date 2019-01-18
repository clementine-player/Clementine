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

#ifndef MTPLOADER_H
#define MTPLOADER_H

#include <memory>

#include <QObject>
#include <QUrl>

class ConnectedDevice;
class LibraryBackend;
class TaskManager;

class MtpLoader : public QObject {
  Q_OBJECT

 public:
  MtpLoader(const QUrl& url, TaskManager* task_manager, LibraryBackend* backend,
            std::shared_ptr<ConnectedDevice> device);
  ~MtpLoader();

 public slots:
  void LoadDatabase();

signals:
  void Error(const QString& message);
  void TaskStarted(int task_id);
  void LoadFinished(bool success);

 private:
  bool TryLoad();

 private:
  std::shared_ptr<ConnectedDevice> device_;
  QThread* original_thread_;

  QUrl url_;
  TaskManager* task_manager_;
  LibraryBackend* backend_;
};

#endif  // MTPLOADER_H
