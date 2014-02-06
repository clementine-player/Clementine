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

#include "internetmodel.h"
#include "somafmservice.h"
#include "somafmurlhandler.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "playlistparsers/playlistparser.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTemporaryFile>

SomaFMUrlHandler::SomaFMUrlHandler(Application* app,
                                   SomaFMServiceBase* service,
                                   QObject* parent)
  : UrlHandler(parent),
    app_(app),
    service_(service),
    task_id_(0)
{
}

QString SomaFMUrlHandler::scheme() const {
  return service_->url_scheme();
}

QIcon SomaFMUrlHandler::icon() const {
  return service_->icon();
}

UrlHandler::LoadResult SomaFMUrlHandler::StartLoading(const QUrl& url) {
  QUrl playlist_url = url;
  playlist_url.setScheme("http");

  // Load the playlist
  QNetworkReply* reply = service_->network()->get(QNetworkRequest(playlist_url));
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));

  if (!task_id_)
    task_id_ = app_->task_manager()->StartTask(tr("Loading stream"));

  return LoadResult(url, LoadResult::WillLoadAsynchronously);
}

void SomaFMUrlHandler::LoadPlaylistFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  app_->task_manager()->SetTaskFinished(task_id_);
  task_id_ = 0;

  QUrl original_url(reply->url());
  original_url.setScheme(scheme());

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: Error handling
    qLog(Error) << reply->errorString();
    emit AsyncLoadComplete(LoadResult(original_url, LoadResult::NoMoreTracks));
    return;
  }

  // Parse the playlist
  PlaylistParser parser(nullptr);
  QList<Song> songs = parser.LoadFromDevice(reply);

  qLog(Info) << "Loading station finished, got" << songs.count() << "songs";

  // Failed to get playlist?
  if (songs.count() == 0) {
    qLog(Error) << "Error loading" << scheme() << "playlist";
    emit AsyncLoadComplete(LoadResult(original_url, LoadResult::NoMoreTracks));
    return;
  }

  emit AsyncLoadComplete(LoadResult(original_url, LoadResult::TrackAvailable,
                                    songs[0].url()));
}
