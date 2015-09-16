/* This file is part of Clementine.
   Copyright 2012-2013, Alan Briolat <alan.briolat@gmail.com>
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

#include <QUrlQuery>

#include "subsonicservice.h"
#include "subsonicurlhandler.h"

SubsonicUrlHandler::SubsonicUrlHandler(SubsonicService* service,
                                       QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult SubsonicUrlHandler::StartLoading(const QUrl& url) {
  if (service_->login_state() != SubsonicService::LoginState_Loggedin)
    return LoadResult(url);

  QUrl newurl = service_->BuildRequestUrl("stream");
  QUrlQuery url_query;
  url_query.addQueryItem("id", url.host());
  newurl.setQuery(url_query);
  return LoadResult(url, LoadResult::TrackAvailable, newurl);
}
