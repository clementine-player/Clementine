/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "moodbarcontroller.h"
#include "moodbarloader.h"
#include "core/application.h"
#include "core/logging.h"
#include "playlist/playlistmanager.h"

MoodbarController::MoodbarController(Application* app, QObject* parent)
  : QObject(parent),
    app_(app)
{
  connect(app_->playlist_manager(),
          SIGNAL(CurrentSongChanged(Song)), SLOT(CurrentSongChanged(Song)));
}

void MoodbarController::CurrentSongChanged(const Song& song) {
  QByteArray data;
  MoodbarPipeline* pipeline = NULL;
  app_->moodbar_loader()->Load(song.url(), &data, &pipeline);
}
