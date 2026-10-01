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

#include "jamendourlhandler.h"

#include "internet/jamendo/jamendoservice.h"
#include "ui/iconloader.h"

JamendoUrlHandler::JamendoUrlHandler(QObject* parent) : UrlHandler(parent) {}

QString JamendoUrlHandler::scheme() const { return JamendoService::kUrlScheme; }

QIcon JamendoUrlHandler::icon() const {
  return IconLoader::Load("jamendo", IconLoader::Provider);
}

UrlHandler::LoadResult JamendoUrlHandler::StartLoading(const QUrl& url) {
  // jamendo://track/<id>
  const QString id = url.path().mid(1);
  if (url.host() != "track" || id.isEmpty()) {
    return LoadResult(url, LoadResult::Error);
  }
  return LoadResult(url, LoadResult::TrackAvailable,
                    JamendoService::StreamUrl(id));
}
