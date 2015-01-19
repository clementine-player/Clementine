/* This file is part of Clementine.
   Copyright 2010, Paweł Bara <keirangtp@gmail.com>
   Copyright 2010-2011, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef CORE_MPRIS_COMMON_H_
#define CORE_MPRIS_COMMON_H_

#include <QDateTime>
#include <QObject>
#include <QStringList>
#include <QVariantMap>

namespace mpris {

inline void AddMetadata(const QString& key, const QString& metadata,
                        QVariantMap* map) {
  if (!metadata.isEmpty()) (*map)[key] = metadata;
}

inline void AddMetadataAsList(const QString& key, const QString& metadata,
                              QVariantMap* map) {
  if (!metadata.isEmpty()) (*map)[key] = QStringList() << metadata;
}

inline void AddMetadata(const QString& key, int metadata, QVariantMap* map) {
  if (metadata > 0) (*map)[key] = metadata;
}

inline void AddMetadata(const QString& key, qint64 metadata, QVariantMap* map) {
  if (metadata > 0) (*map)[key] = metadata;
}

inline void AddMetadata(const QString& key, double metadata, QVariantMap* map) {
  if (metadata != 0.0) (*map)[key] = metadata;
}

inline void AddMetadata(const QString& key, const QDateTime& metadata,
                        QVariantMap* map) {
  if (metadata.isValid()) (*map)[key] = metadata;
}

inline QString AsMPRISDateTimeType(uint time) {
  return time != -1 ? QDateTime::fromTime_t(time).toString(Qt::ISODate) : "";
}

}  // namespace mpris

#endif  // CORE_MPRIS_COMMON_H_
