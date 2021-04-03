/* This file is part of Clementine.
   Copyright 2021, Fabio Bas <ctrlaltca@gmail.com>

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

#include "core/application.h"
#include "core/taskmanager.h"
#include "radiobrowserservice.h"

RadioBrowserUrlHandler::RadioBrowserUrlHandler(Application* app,
                                               RadioBrowserService* service,
                                               QObject* parent)
    : UrlHandler(parent), app_(app), service_(service) {
  connect(service_, &RadioBrowserService::StationUrlResolveFailed, this,
          &RadioBrowserUrlHandler::LoadStationFailed);
  connect(service_, &RadioBrowserService::StreamMetadataFound, this,
          &RadioBrowserUrlHandler::LoadStationFinished);
}

QString RadioBrowserUrlHandler::scheme() const {
  return RadioBrowserService::kSchemeName;
}

QIcon RadioBrowserUrlHandler::icon() const { return service_->icon(); }

UrlHandler::LoadResult RadioBrowserUrlHandler::StartLoading(const QUrl& url) {
  service_->ResolveStationUrl(url);
  return LoadResult(url, LoadResult::WillLoadAsynchronously);
}

void RadioBrowserUrlHandler::LoadStationFailed(const QUrl& original_url) {
  qLog(Error) << "Error loading" << original_url;
  emit AsyncLoadComplete(LoadResult(original_url, LoadResult::NoMoreTracks));
}

void RadioBrowserUrlHandler::LoadStationFinished(const QUrl& original_url,
                                                 const Song& song) {
  emit AsyncLoadComplete(
      LoadResult(original_url, LoadResult::TrackAvailable, song.url()));
}