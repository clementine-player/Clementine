/* This file is part of Clementine.
   Copyright 2010-2012, 2014, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
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

#include "currentartloader.h"

#include <QDir>
#include <QTemporaryFile>
#include <QUrl>

#include "core/application.h"
#include "covers/albumcoverloader.h"
#include "playlist/playlistmanager.h"

CurrentArtLoader::CurrentArtLoader(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      temp_file_pattern_(QDir::tempPath() + "/clementine-art-XXXXXX.jpg"),
      id_(0) {
  options_.scale_output_image_ = false;
  options_.pad_output_image_ = false;
  options_.default_output_image_ = QImage(":nocover.png");

  connect(app_->album_cover_loader(), SIGNAL(ImageLoaded(quint64, QImage)),
          SLOT(TempArtLoaded(quint64, QImage)));

  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(LoadArt(Song)));
}

CurrentArtLoader::~CurrentArtLoader() {}

void CurrentArtLoader::LoadArt(const Song& song) {
  last_song_ = song;
  id_ = app_->album_cover_loader()->LoadImageAsync(options_, last_song_);
}

void CurrentArtLoader::TempArtLoaded(quint64 id, const QImage& image) {
  if (id != id_) return;
  id_ = 0;

  QString uri;
  QString thumbnail_uri;
  QImage thumbnail;

  if (!image.isNull()) {
    temp_art_.reset(new QTemporaryFile(temp_file_pattern_));
    temp_art_->setAutoRemove(true);
    temp_art_->open();
    image.save(temp_art_->fileName(), "JPEG");

    // Scale the image down to make a thumbnail.  It's a bit crap doing it here
    // since it's the GUI thread, but the alternative is hard.
    temp_art_thumbnail_.reset(new QTemporaryFile(temp_file_pattern_));
    temp_art_thumbnail_->open();
    temp_art_thumbnail_->setAutoRemove(true);
    thumbnail = image.scaledToHeight(120, Qt::SmoothTransformation);
    thumbnail.save(temp_art_thumbnail_->fileName(), "JPEG");

    uri = "file://" + temp_art_->fileName();
    thumbnail_uri = "file://" + temp_art_thumbnail_->fileName();
  }

  emit ArtLoaded(last_song_, uri, image);
  emit ThumbnailLoaded(last_song_, thumbnail_uri, thumbnail);
}
