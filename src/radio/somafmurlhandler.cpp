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

#include "radiomodel.h"
#include "somafmservice.h"
#include "somafmurlhandler.h"
#include "core/logging.h"
#include "core/taskmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTemporaryFile>

SomaFMUrlHandler::SomaFMUrlHandler(SomaFMService* service, QObject* parent)
  : UrlHandler(parent),
    service_(service),
    task_id_(0)
{
}

UrlHandler_LoadResult SomaFMUrlHandler::StartLoading(const QUrl& url) {
  QUrl playlist_url = url;
  playlist_url.setScheme("http");

  // Load the playlist
  QNetworkReply* reply = service_->network()->get(QNetworkRequest(playlist_url));
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));

  if (!task_id_)
    task_id_ = service_->model()->task_manager()->StartTask(tr("Loading stream"));

  return UrlHandler_LoadResult(url, UrlHandler_LoadResult::WillLoadAsynchronously);
}

void SomaFMUrlHandler::LoadPlaylistFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  service_->model()->task_manager()->SetTaskFinished(task_id_);
  task_id_ = 0;

  QUrl original_url(reply->url());
  original_url.setScheme("somafm");

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: Error handling
    qLog(Error) << reply->errorString();
    emit AsyncLoadComplete(UrlHandler_LoadResult(
        original_url, UrlHandler_LoadResult::NoMoreTracks));
    return;
  }

  // TODO: Replace with some more robust .pls parsing :(
  QTemporaryFile temp_file;
  temp_file.open();
  temp_file.write(reply->readAll());
  temp_file.flush();

  QSettings s(temp_file.fileName(), QSettings::IniFormat);
  s.beginGroup("playlist");

  emit AsyncLoadComplete(UrlHandler_LoadResult(
      original_url, UrlHandler_LoadResult::TrackAvailable,
      s.value("File1").toString()));
}
