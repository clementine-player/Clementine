/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <davidsansome@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "icecastbackend.h"

#include <QSqlQuery>
#include <QVariant>

#include "core/database.h"
#include "core/scopedtransaction.h"

const char* IcecastBackend::kTableName = "icecast_stations";

IcecastBackend::IcecastBackend(QObject* parent) : QObject(parent) {}

void IcecastBackend::Init(Database* db) { db_ = db; }

QStringList IcecastBackend::GetGenresAlphabetical(const QString& filter) {
  QStringList ret;
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db = db_->Connect();

  QString where = filter.isEmpty() ? "" : "WHERE name LIKE :filter";

  QString sql = QString("SELECT DISTINCT genre FROM %1 %2 ORDER BY genre")
                    .arg(kTableName, where);

  QSqlQuery q(sql, db);
  if (!filter.isEmpty()) {
    q.bindValue(":filter", QString("%" + filter + "%"));
  }

  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    ret << q.value(0).toString();
  }
  return ret;
}

QStringList IcecastBackend::GetGenresByPopularity(const QString& filter) {
  QStringList ret;
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db = db_->Connect();

  QString where = filter.isEmpty() ? "" : "WHERE name LIKE :filter";

  QString sql = QString(
                    "SELECT genre, COUNT(*) AS count FROM %1 "
                    " %2"
                    " GROUP BY genre"
                    " ORDER BY count DESC").arg(kTableName, where);
  QSqlQuery q(sql, db);
  if (!filter.isEmpty()) {
    q.bindValue(":filter", QString("%" + filter + "%"));
  }

  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    ret << q.value(0).toString();
  }
  return ret;
}

IcecastBackend::StationList IcecastBackend::GetStations(const QString& filter,
                                                        const QString& genre) {
  StationList ret;
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db = db_->Connect();

  QStringList where_clauses;
  QStringList bound_items;

  if (!genre.isEmpty()) {
    where_clauses << "genre = :genre";
    bound_items << genre;
  }
  if (!filter.isEmpty()) {
    where_clauses << "name LIKE :filter";
    bound_items << "%" + filter + "%";
  }

  QString sql = QString(
                    "SELECT name, url, mime_type, bitrate, channels,"
                    "       samplerate, genre"
                    " FROM %1").arg(kTableName);

  if (!where_clauses.isEmpty()) {
    sql += " WHERE " + where_clauses.join(" AND ");
  }
  QSqlQuery q(sql, db);
  for (const QString& value : bound_items) {
    q.addBindValue(value);
  }

  q.exec();
  if (db_->CheckErrors(q)) return ret;

  while (q.next()) {
    Station station;
    station.name = q.value(0).toString();
    station.url = QUrl(q.value(1).toString());
    station.mime_type = q.value(2).toString();
    station.bitrate = q.value(3).toInt();
    station.channels = q.value(4).toInt();
    station.samplerate = q.value(5).toInt();
    station.genre = q.value(6).toString();
    ret << station;
  }
  return ret;
}

bool IcecastBackend::IsEmpty() {
  QMutexLocker l(db_->Mutex());
  QSqlDatabase db = db_->Connect();
  QSqlQuery q(QString("SELECT ROWID FROM %1 LIMIT 1").arg(kTableName), db);
  q.exec();
  return !q.next();
}

void IcecastBackend::ClearAndAddStations(const StationList& stations) {
  {
    QMutexLocker l(db_->Mutex());
    QSqlDatabase db = db_->Connect();
    ScopedTransaction t(&db);

    // Remove all existing items
    QSqlQuery q(QString("DELETE FROM %1").arg(kTableName), db);
    q.exec();
    if (db_->CheckErrors(q)) return;

    q = QSqlQuery(
        QString(
            "INSERT INTO %1 (name, url, mime_type, bitrate,"
            "                channels, samplerate, genre)"
            " VALUES (:name, :url, :mime_type, :bitrate,"
            "         :channels, :samplerate, :genre)").arg(kTableName),
        db);

    // Add these ones
    for (const Station& station : stations) {
      q.bindValue(":name", station.name);
      q.bindValue(":url", station.url);
      q.bindValue(":mime_type", station.mime_type);
      q.bindValue(":bitrate", station.bitrate);
      q.bindValue(":channels", station.channels);
      q.bindValue(":samplerate", station.samplerate);
      q.bindValue(":genre", station.genre);
      q.exec();
      if (db_->CheckErrors(q)) return;
    }

    t.Commit();
  }

  emit DatabaseReset();
}

Song IcecastBackend::Station::ToSong() const {
  Song ret;
  ret.set_valid(true);
  ret.set_title(name);
  ret.set_url(url);
  ret.set_bitrate(bitrate);
  ret.set_samplerate(samplerate);
  ret.set_genre(genre);
  ret.set_filetype(Song::Type_Stream);
  return ret;
}
