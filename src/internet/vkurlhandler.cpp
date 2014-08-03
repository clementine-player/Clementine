/* This file is part of Clementine.
   Copyright 2013, Vlad Maltsev <shedwardx@gmail.com>

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

#include "vkurlhandler.h"

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"

#include "vkservice.h"
#include "vkmusiccache.h"

VkUrlHandler::VkUrlHandler(VkService* service, QObject* parent)
  : UrlHandler(parent),
    service_(service) {
}

UrlHandler::LoadResult VkUrlHandler::StartLoading(const QUrl& url) {
  QStringList args = url.path().split("/");
  LoadResult result;

  if (args.size() < 2) {
    qLog(Error) << "Invalid Vk.com URL: " << url
                << "Url format should be vk://<source>/<id>."
                << "For example vk://song/61145020_166946521/Daughtry/Gone Too Soon";
  } else {
    QString action = url.host();

    if (action == "song") {
      result = service_->GetSongResult(url);
    } else if (action == "group") {
      result = service_->GetGroupNextSongUrl(url);
    } else {
      qLog(Error) << "Invalid vk.com url action:" << action;
    }
  }

  return result;
}

void VkUrlHandler::TrackSkipped() {
  service_->SongSkiped();
}

UrlHandler::LoadResult VkUrlHandler::LoadNext(const QUrl& url) {
  if (url.host() == "group") {
    return StartLoading(url);
  } else {
    return LoadResult();
  }
}
