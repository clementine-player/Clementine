/* This file is part of Clementine.
   Copyright 2011-2013, David Sansome <me@davidsansome.com>
   Copyright 2012, Olaf Christ <olafc81@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "radiobrowserurlhandler.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTemporaryFile>

#include "core/application.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "radiobrowserservice.h"
#include "internet/core/internetmodel.h"
#include "playlistparsers/playlistparser.h"

RadioBrowserUrlHandler::RadioBrowserUrlHandler(
    Application* app, RadioBrowserServiceBase* service, QObject* parent)
    : UrlHandler(parent), app_(app), service_(service), task_id_(0) {}

QString RadioBrowserUrlHandler::scheme() const {
  return service_->url_scheme();
}

QIcon RadioBrowserUrlHandler::icon() const { return service_->icon(); }

UrlHandler::LoadResult RadioBrowserUrlHandler::StartLoading(
    const QUrl& url) {
  QUrl playlist_url = url;
  playlist_url.setScheme("https");

  // Load the playlist
  QNetworkReply* reply =
      service_->network()->get(QNetworkRequest(playlist_url));
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));

  if (!task_id_)
    task_id_ = app_->task_manager()->StartTask(tr("Loading stream"));

  return LoadResult(url, LoadResult::WillLoadAsynchronously);
}

void RadioBrowserUrlHandler::LoadPlaylistFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  app_->task_manager()->SetTaskFinished(task_id_);
  task_id_ = 0;

  QUrl original_url(reply->url());
  original_url.setScheme(scheme());

  if (reply->error() != QNetworkReply::NoError) {
    // TODO((David Sansome): Error handling
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

  emit AsyncLoadComplete(
      LoadResult(original_url, LoadResult::TrackAvailable, songs[0].url()));
}
