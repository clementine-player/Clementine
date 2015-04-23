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

#include "devicedatabasebackend.h"
#include "core/database.h"
#include "core/scopedtransaction.h"

#include <QFile>
#include <QSqlQuery>
#include <QVariant>

const int DeviceDatabaseBackend::kDeviceSchemaVersion = 0;

DeviceDatabaseBackend::DeviceDatabaseBackend(QObject* parent)
    : QObject(parent) {}

void DeviceDatabaseBackend::Init(Database* db) { db_ = db; }

DeviceDatabaseBackend::DeviceList DeviceDatabaseBackend::GetAllDevices() {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  DeviceList ret;

  QSqlQuery q(db);
  q.prepare("SELECT ROWID, unique_id, friendly_name, size, icon,"
      "   transcode_mode, transcode_format"
      " FROM devices");
  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    Device dev;
    dev.id_ = q.value(0).toInt();
    dev.unique_id_ = q.value(1).toString();
    dev.friendly_name_ = q.value(2).toString();
    dev.size_ = q.value(3).toLongLong();
    dev.icon_name_ = q.value(4).toString();
    dev.transcode_mode_ = MusicStorage::TranscodeMode(q.value(5).toInt());
    dev.transcode_format_ = Song::FileType(q.value(6).toInt());
    ret << dev;
  }
  return ret;
}

int DeviceDatabaseBackend::AddDevice(const Device& device) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  ScopedTransaction t(&db);

  // Insert the device into the devices table
  QSqlQuery q(db);
  q.prepare("INSERT INTO devices ("
      "   unique_id, friendly_name, size, icon,"
      "   transcode_mode, transcode_format)"
      " VALUES (:unique_id, :friendly_name, :size, :icon,"
      "   :transcode_mode, :transcode_format)");
  q.bindValue(":unique_id", device.unique_id_);
  q.bindValue(":friendly_name", device.friendly_name_);
  q.bindValue(":size", device.size_);
  q.bindValue(":icon", device.icon_name_);
  q.bindValue(":transcode_mode", device.transcode_mode_);
  q.bindValue(":transcode_format", device.transcode_format_);
  q.exec();
  if (db_->CheckErrors(q)) return -1;
  int id = q.lastInsertId().toInt();

  // Create the songs tables for the device
  QString filename(":schema/device-schema.sql");
  QFile schema_file(filename);
  if (!schema_file.open(QIODevice::ReadOnly))
    qFatal("Couldn't open schema file %s", filename.toUtf8().constData());
  QString schema = QString::fromUtf8(schema_file.readAll());
  schema.replace("%deviceid", QString::number(id));

  db_->ExecSchemaCommands(db, schema, 0, true);

  t.Commit();
  return id;
}

void DeviceDatabaseBackend::RemoveDevice(int id) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  ScopedTransaction t(&db);

  // Remove the device from the devices table
  QSqlQuery q(db);
  q.prepare("DELETE FROM devices WHERE ROWID=:id");
  q.bindValue(":id", id);
  q.exec();
  if (db_->CheckErrors(q)) return;

  // Remove the songs tables for the device
  db.exec(QString("DROP TABLE device_%1_songs").arg(id));
  db.exec(QString("DROP TABLE device_%1_fts").arg(id));
  db.exec(QString("DROP TABLE device_%1_directories").arg(id));
  db.exec(QString("DROP TABLE device_%1_subdirectories").arg(id));

  t.Commit();
}

void DeviceDatabaseBackend::SetDeviceOptions(int id,
                                             const QString& friendly_name,
                                             const QString& icon_name,
                                             MusicStorage::TranscodeMode mode,
                                             Song::FileType format) {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db(db_->Connect());

  QSqlQuery q(db);
  q.prepare("UPDATE devices"
      " SET friendly_name=:friendly_name,"
      "     icon=:icon_name,"
      "     transcode_mode=:transcode_mode,"
      "     transcode_format=:transcode_format"
      " WHERE ROWID=:id");
  q.bindValue(":friendly_name", friendly_name);
  q.bindValue(":icon_name", icon_name);
  q.bindValue(":transcode_mode", mode);
  q.bindValue(":transcode_format", format);
  q.bindValue(":id", id);
  q.exec();
  db_->CheckErrors(q);
}
