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
#include "moodbarpipeline.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/player.h"
#include "playlist/playlistmanager.h"

MoodbarController::MoodbarController(Application* app, QObject* parent)
    : QObject(parent), app_(app) {
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(CurrentSongChanged(Song)));
  connect(app_->player(), SIGNAL(Stopped()), SLOT(PlaybackStopped()));
}

void MoodbarController::CurrentSongChanged(const Song& song) {
  QByteArray data;
  MoodbarPipeline* pipeline = nullptr;
  const MoodbarLoader::Result result =
      app_->moodbar_loader()->Load(song.url(), &data, &pipeline);

  switch (result) {
    case MoodbarLoader::CannotLoad:
      emit CurrentMoodbarDataChanged(QByteArray());
      break;

    case MoodbarLoader::Loaded:
      emit CurrentMoodbarDataChanged(data);
      break;

    case MoodbarLoader::WillLoadAsync:
      // Emit an empty array for now so the GUI reverts to a normal progress
      // bar.  Our slot will be called when the data is actually loaded.
      emit CurrentMoodbarDataChanged(QByteArray());

      NewClosure(pipeline, SIGNAL(Finished(bool)), this,
                 SLOT(AsyncLoadComplete(MoodbarPipeline*, QUrl)), pipeline,
                 song.url());
      break;
  }
}

void MoodbarController::PlaybackStopped() {
  emit CurrentMoodbarDataChanged(QByteArray());
}

void MoodbarController::AsyncLoadComplete(MoodbarPipeline* pipeline,
                                          const QUrl& url) {
  // Is this song still playing?
  PlaylistItemPtr current_item = app_->player()->GetCurrentItem();
  if (current_item && current_item->Url() != url) {
    return;
  }
  // Did we stop the song?
  switch(app_->player()->GetState()) {
    case Engine::Error:
    case Engine::Empty:
    case Engine::Idle:
      return;

    default:
      break;
  }

  emit CurrentMoodbarDataChanged(pipeline->data());
}
