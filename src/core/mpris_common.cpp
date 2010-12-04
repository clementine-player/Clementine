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
#include "mpris_common.h"

#include <QDir>
#include <QTemporaryFile>

namespace mpris {

ArtLoader::ArtLoader(QObject* parent)
  : QObject(parent),
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    id_(0)
{
  cover_loader_->Start();
  connect(cover_loader_, SIGNAL(Initialised()), SLOT(Initialised()));
}

ArtLoader::~ArtLoader() {
}

void ArtLoader::Initialised() {
  cover_loader_->Worker()->SetPadOutputImage(true);
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

  if (!image.isNull()) {
    temp_art_.reset(new QTemporaryFile(QDir::tempPath() + "/clementine-art-XXXXXX.jpg"));
    temp_art_->open();
    image.save(temp_art_->fileName(), "JPEG");

    uri = "file://" + temp_art_->fileName();
  }

  emit ArtLoaded(last_song_, uri);
}


} // namespace mpris
