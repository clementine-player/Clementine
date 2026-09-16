/* This file is part of Clementine.
   Copyright 2026

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

#include "megaurlhandler.h"

#include "internet/mega/megaservice.h"

MegaUrlHandler::MegaUrlHandler(MegaService* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult MegaUrlHandler::StartLoading(const QUrl& url) {
  QUrl streaming_url = service_->GetStreamingUrlFromSongId(url);
  if (streaming_url.isValid()) {
    return LoadResult(url, LoadResult::TrackAvailable, streaming_url);
  }
  return LoadResult(url, LoadResult::Error);
}
