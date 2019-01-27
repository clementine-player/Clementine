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

#ifndef DEVICEPROPERTIES_H
#define DEVICEPROPERTIES_H

#include <QDialog>
#include <QFuture>
#include <QPersistentModelIndex>

#include "core/song.h"

class DeviceManager;
class Ui_DeviceProperties;

class DeviceProperties : public QDialog {
  Q_OBJECT

 public:
  DeviceProperties(QWidget* parent = nullptr);
  ~DeviceProperties();

  void SetDeviceManager(DeviceManager* manager);
  void ShowDevice(QModelIndex idx);

 public slots:
  void accept();

 private:
  void UpdateHardwareInfo();
  void AddHardwareInfo(int row, const QString& key, const QString& value);
  void UpdateFormats();

 private slots:
  void ModelChanged();
  void OpenDevice();
  void UpdateFormatsFinished(QFuture<bool> future);

 private:
  Ui_DeviceProperties* ui_;

  DeviceManager* manager_;
  QPersistentModelIndex index_;

  bool updating_formats_;
  QList<Song::FileType> supported_formats_;
};

#endif  // DEVICEPROPERTIES_H
