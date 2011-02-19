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

#ifndef ARTLOADER_H
#define ARTLOADER_H

#include "backgroundthread.h"
#include "song.h"

#include <QObject>

#include <boost/scoped_ptr.hpp>

class AlbumCoverLoader;

class QImage;
class QTemporaryFile;

class ArtLoader : public QObject {
  Q_OBJECT

public:
  ArtLoader(QObject* parent = 0);
  ~ArtLoader();

public slots:
  void LoadArt(const Song& song);

signals:
  void ArtLoaded(const Song& song, const QString& uri, const QImage& image);
  void ThumbnailLoaded(const Song& song, const QString& uri, const QImage& image);

private slots:
  void Initialised();
  void TempArtLoaded(quint64 id, const QImage& image);

private:
  QString temp_file_pattern_;

  boost::scoped_ptr<QTemporaryFile> temp_art_;
  boost::scoped_ptr<QTemporaryFile> temp_art_thumbnail_;
  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  quint64 id_;

  Song last_song_;
};

#endif // ARTLOADER_H
