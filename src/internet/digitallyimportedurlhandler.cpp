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

#include "digitallyimportedservicebase.h"
#include "digitallyimportedurlhandler.h"
#include "internetmodel.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "playlistparsers/playlistparser.h"

DigitallyImportedUrlHandler::DigitallyImportedUrlHandler(
    Application* app, DigitallyImportedServiceBase* service)
    : UrlHandler(service), app_(app), service_(service), task_id_(-1) {}

QString DigitallyImportedUrlHandler::scheme() const {
  return service_->api_service_name();
}

QIcon DigitallyImportedUrlHandler::icon() const {
  if (scheme() == "di") {
    return QIcon(":providers/digitallyimported.png");
  } else if (scheme() == "sky") {
    return QIcon(":providers/skyfm.png");
  } else if (scheme() == "jazzradio") {
    return QIcon(":providers/jazzradio.png");
  } else if (scheme() == "rockradio") {
    return QIcon(":providers/rockradio.png");
  }
  return QIcon();
}

UrlHandler::LoadResult DigitallyImportedUrlHandler::StartLoading(
    const QUrl& url) {
  LoadResult ret(url);
  if (task_id_ != -1) {
    return ret;
  }

  // Start loading the station
  const QString key = url.host();
  qLog(Info) << "Loading station" << key;
  service_->LoadStation(key);

  // Save the URL so we can emit it in the finished signal later
  last_original_url_ = url;

  // Tell the user what's happening
  task_id_ = app_->task_manager()->StartTask(tr("Loading stream"));

  ret.type_ = LoadResult::WillLoadAsynchronously;
  return ret;
}

void DigitallyImportedUrlHandler::LoadPlaylistFinished(QIODevice* device) {
  if (task_id_ == -1) {
    return;
  }

  // Stop the spinner in the status bar
  CancelTask();

  // Try to parse the playlist
  PlaylistParser parser(nullptr);
  QList<Song> songs = parser.LoadFromDevice(device);

  qLog(Info) << "Loading station finished, got" << songs.count() << "songs";

  // Failed to get playlist?
  if (songs.count() == 0) {
    service_->StreamError(tr("Error loading di.fm playlist"));
    return;
  }

  emit AsyncLoadComplete(LoadResult(
      last_original_url_, LoadResult::TrackAvailable, songs[0].url()));
}

void DigitallyImportedUrlHandler::CancelTask() {
  app_->task_manager()->SetTaskFinished(task_id_);
  task_id_ = -1;
}
