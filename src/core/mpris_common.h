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

#ifndef MPRIS_COMMON_H
#define MPRIS_COMMON_H

#include "backgroundthread.h"
#include "song.h"

#include <QDateTime>
#include <QObject>
#include <QStringList>
#include <QVariantMap>

#include <boost/scoped_ptr.hpp>

class AlbumCoverLoader;

class QImage;
class QTemporaryFile;

namespace mpris {

class ArtLoader : public QObject {
  Q_OBJECT

public:
  ArtLoader(QObject* parent);
  ~ArtLoader();

public slots:
  void LoadArt(const Song& song);

signals:
  void ArtLoaded(const Song& song, const QString& uri);

private slots:
  void Initialised();
  void TempArtLoaded(quint64 id, const QImage& image);

private:
  boost::scoped_ptr<QTemporaryFile> temp_art_;
  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  quint64 id_;

  Song last_song_;
};


inline void AddMetadata(const QString& key, const QString& metadata, QVariantMap* map) {
  if (!metadata.isEmpty())   (*map)[key] = metadata;
}

inline void AddMetadata(const QString& key, int metadata, QVariantMap* map) {
  if (metadata > 0)          (*map)[key] = metadata;
}

inline void AddMetadata(const QString& key, const QDateTime& metadata, QVariantMap* map) {
  if (metadata.isValid())    (*map)[key] = metadata;
}

inline void AddMetadata(const QString &key, const QStringList& metadata, QVariantMap *map) {
  if (!metadata.isEmpty())   (*map)[key] = metadata;
}

} // namespace mpris

#endif // MPRIS_COMMON_H
