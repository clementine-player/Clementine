/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
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

#include "skydriveurlhandler.h"

#include "skydriveservice.h"

SkydriveUrlHandler::SkydriveUrlHandler(SkydriveService* service,
                                       QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult SkydriveUrlHandler::StartLoading(const QUrl& url) {
  QString file_id(url.path());
  QUrl real_url = service_->GetStreamingUrlFromSongId(file_id);
  LoadResult res(url, LoadResult::TrackAvailable, real_url);
  res.auth_header_ = service_->GetAuthHeader();
  return res;
}

QString SkydriveUrlHandler::scheme() const { return service_->GetScheme(); }
