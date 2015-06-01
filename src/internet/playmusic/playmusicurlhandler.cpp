/* This file is part of Clementine.
   Copyright 2015, Marco Kirchner <kirchnermarco@gmail.com>
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

#include "playmusicurlhandler.h"

#include "playmusicservice.h"

PlayMusicUrlHandler::PlayMusicUrlHandler(PlayMusicService* service,
                                             QObject* parent)
    : UrlHandler(parent), service_(service) {
    connect(service, SIGNAL(LoadStreamUrlComplete(QUrl,QString)), this, SLOT(LoadStreamUrlFinished(QUrl,QString)));
}

UrlHandler::LoadResult PlayMusicUrlHandler::StartLoading(const QUrl& url) {
  QString file_id = url.host();
  service_->LoadStreamUrl(file_id);
  return LoadResult(url, LoadResult::WillLoadAsynchronously);
}

void PlayMusicUrlHandler::LoadStreamUrlFinished(const QUrl &url, const QString &id)
{
    AsyncLoadComplete(LoadResult(QUrl("playmusic://" + id), LoadResult::TrackAvailable, url));
}
