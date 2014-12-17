/* This file is part of Clementine.
   Copyright 2014, Chocobozzz <djidane14ff@hotmail.fr>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "seafileurlhandler.h"

#include "seafileservice.h"

SeafileUrlHandler::SeafileUrlHandler(SeafileService* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult SeafileUrlHandler::StartLoading(const QUrl& url) {
  QString file_library_and_path = url.path();
  QRegExp reg("/([^/]+)(/.*)$");

  if (reg.indexIn(file_library_and_path) == -1) {
    qLog(Debug) << "Can't find repo and file path in " << url;
  }

  QString library = reg.cap(1);
  QString filepath = reg.cap(2);

  QUrl real_url = service_->GetStreamingUrlFromSongId(library, filepath);

  return LoadResult(url, LoadResult::TrackAvailable, real_url);
}
