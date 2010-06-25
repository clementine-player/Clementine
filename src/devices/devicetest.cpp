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

#include "devicetest.h"
#include "filesystemdeviceengine.h"

#include <QtDebug>

DeviceTest::DeviceTest(QObject *parent)
  : QObject(parent)
{
  DeviceEngine* engine = new FilesystemDeviceEngine;
  engines_ << engine;
  connect(engine, SIGNAL(DeviceAdded(QString)), SLOT(DeviceAdded(QString)));
  connect(engine, SIGNAL(DeviceRemoved(QString)), SLOT(DeviceRemoved(QString)));
  connect(engine, SIGNAL(DeviceChanged(QString)), SLOT(DeviceChanged(QString)));

  engine->Start();
}

DeviceTest::~DeviceTest() {
  qDeleteAll(engines_);
}

void DeviceTest::DeviceAdded(const QString &id) {
  DeviceEngine* engine = qobject_cast<DeviceEngine*>(sender());

  qDebug() << "Device added:" << id;
  for (int i=0 ; i<FilesystemDeviceEngine::LastFilesystemDeviceEngineField ; ++i) {
    qDebug() << i << engine->DeviceInfo(id, i);
  }
}

void DeviceTest::DeviceRemoved(const QString &id) {
  qDebug() << "Device removed:" << id;
}

void DeviceTest::DeviceChanged(const QString &id) {
  DeviceEngine* engine = qobject_cast<DeviceEngine*>(sender());

  qDebug() << "Device changed:" << id;
  for (int i=0 ; i<FilesystemDeviceEngine::LastFilesystemDeviceEngineField ; ++i) {
    qDebug() << i << engine->DeviceInfo(id, i);
  }
}
