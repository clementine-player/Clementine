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

#include "albumcoverloader.h"
#include "artloader.h"

#include <QDir>
#include <QTemporaryFile>

ArtLoader::ArtLoader(QObject* parent)
  : QObject(parent),
    temp_file_pattern_(QDir::tempPath() + "/clementine-art-XXXXXX.jpg"),
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    id_(0)
{
  cover_loader_->Start();
  connect(cover_loader_, SIGNAL(Initialised()), SLOT(Initialised()));
}

ArtLoader::~ArtLoader() {
}

void ArtLoader::Initialised() {
  cover_loader_->Worker()->SetScaleOutputImage(false);
  cover_loader_->Worker()->SetPadOutputImage(false);
  cover_loader_->Worker()->SetDefaultOutputImage(QImage(":nocover.png"));
  connect(cover_loader_->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(TempArtLoaded(quint64,QImage)));
}

void ArtLoader::LoadArt(const Song& song) {
  last_song_ = song;
  id_ = cover_loader_->Worker()->LoadImageAsync(song);
}

void ArtLoader::TempArtLoaded(quint64 id, const QImage& image) {
  if (id != id_)
    return;
  id_ = 0;

  QString uri;
  QString thumbnail_uri;
  QImage thumbnail;

  if (!image.isNull()) {
    temp_art_.reset(new QTemporaryFile(temp_file_pattern_));
    temp_art_->open();
    image.save(temp_art_->fileName(), "JPEG");

    // Scale the image down to make a thumbnail.  It's a bit crap doing it here
    // since it's the GUI thread, but the alternative is hard.
    temp_art_thumbnail_.reset(new QTemporaryFile(temp_file_pattern_));
    temp_art_thumbnail_->open();
    thumbnail = image.scaledToHeight(120, Qt::SmoothTransformation);
    thumbnail.save(temp_art_thumbnail_->fileName(), "JPEG");

    uri = "file://" + temp_art_->fileName();
    thumbnail_uri = "file://" + temp_art_thumbnail_->fileName();
  }

  emit ArtLoaded(last_song_, uri, image);
  emit ThumbnailLoaded(last_song_, thumbnail_uri, thumbnail);
}
