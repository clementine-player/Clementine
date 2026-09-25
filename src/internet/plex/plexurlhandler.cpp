/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "plexurlhandler.h"

#include <QUrlQuery>

#include "core/logging.h"
#include "internet/plex/plexparser.h"
#include "internet/plex/plexservice.h"
#include "ui/iconloader.h"

PlexUrlHandler::PlexUrlHandler(PlexService* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

QString PlexUrlHandler::scheme() const { return PlexParser::kScheme; }

QIcon PlexUrlHandler::icon() const {
  return IconLoader::Load("plex", IconLoader::Provider);
}

UrlHandler::LoadResult PlexUrlHandler::StartLoading(const QUrl& url) {
  if (service_->login_state() != PlexService::LoginState_LoggedIn) {
    return LoadResult(url, LoadResult::Error);
  }

  if (PlexParser::MachineIdFromUrl(url) != service_->machine_id()) {
    qLog(Warning) << "Plex track" << url << "is from a server we're not using";
    return LoadResult(url, LoadResult::Error);
  }

  const QString part_key = PlexParser::PartKeyFromUrl(url);
  if (!part_key.startsWith('/')) {
    return LoadResult(url, LoadResult::Error);
  }

  QUrl media_url(service_->base_url().toString() + part_key);
  QUrlQuery query;
  query.addQueryItem("X-Plex-Token", service_->server_token());
  media_url.setQuery(query);
  return LoadResult(url, LoadResult::TrackAvailable, media_url);
}
