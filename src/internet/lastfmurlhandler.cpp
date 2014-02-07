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

#include "lastfmservice.h"
#include "lastfmurlhandler.h"

LastFMUrlHandler::LastFMUrlHandler(LastFMService* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult LastFMUrlHandler::StartLoading(const QUrl& url) {
  if (!service_->IsAuthenticated()) return LoadResult();

  service_->Tune(url);
  return LoadResult(url, LoadResult::WillLoadAsynchronously);
}

void LastFMUrlHandler::TunerTrackAvailable() {
  emit AsyncLoadComplete(LoadNext(service_->last_url_));
}

void LastFMUrlHandler::TunerError() {
  emit AsyncLoadComplete(
      LoadResult(service_->last_url_, LoadResult::NoMoreTracks));
}

UrlHandler::LoadResult LastFMUrlHandler::LoadNext(const QUrl& url) {
  const QUrl media_url = service_->DeququeNextMediaUrl();
  if (media_url.isEmpty()) {
    return LoadResult();
  }
  return LoadResult(url, LoadResult::TrackAvailable, media_url);
}
