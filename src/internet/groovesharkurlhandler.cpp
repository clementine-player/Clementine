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

#include "groovesharkservice.h"
#include "groovesharkurlhandler.h"

#include "core/logging.h"

GrooveSharkUrlHandler::GrooveSharkUrlHandler(GrooveSharkService* service, QObject* parent)
  : UrlHandler(parent),
    service_(service) {
}

UrlHandler::LoadResult GrooveSharkUrlHandler::StartLoading(const QUrl& url) {
  QString song_id = url.toString().remove("grooveshark://");
  QUrl streaming_url = service_->GetStreamingUrlFromSongId(song_id);
  qLog(Debug) << "GrooveShark Streaming URL: " << streaming_url;
  return LoadResult(url, LoadResult::TrackAvailable, streaming_url);
}


