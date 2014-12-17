/* This file is part of Clementine.
   Copyright 2010, 2012, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_ICECASTBACKEND_H_
#define INTERNET_ICECASTBACKEND_H_

#include "core/song.h"

#include <QObject>
#include <QUrl>

class Database;

class IcecastBackend : public QObject {
  Q_OBJECT

 public:
  explicit IcecastBackend(QObject* parent = nullptr);
  void Init(Database* db);

  static const char* kTableName;

  struct Station {
    Station() : bitrate(0), channels(0), samplerate(0) {}

    QString name;
    QUrl url;
    QString mime_type;
    int bitrate;
    int channels;
    int samplerate;
    QString genre;

    Song ToSong() const;
  };
  typedef QList<Station> StationList;

  QStringList GetGenresAlphabetical(const QString& filter = QString());
  QStringList GetGenresByPopularity(const QString& filter = QString());
  StationList GetStations(const QString& filter = QString(),
                          const QString& genre = QString());

  void ClearAndAddStations(const StationList& stations);

  bool IsEmpty();

 signals:
  void DatabaseReset();

 private:
  Database* db_;
};

#endif  // INTERNET_ICECASTBACKEND_H_
