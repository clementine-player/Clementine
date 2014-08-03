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

#ifndef VKURLHANDLER_H
#define VKURLHANDLER_H

#include "core/urlhandler.h"
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class VkService;
class VkMusicCache;

class VkUrlHandler : public UrlHandler {
  Q_OBJECT
public:
  VkUrlHandler(VkService* service, QObject* parent);
  QString scheme() const { return "vk"; }
  QIcon icon() const { return QIcon(":providers/vk.png"); }
  LoadResult StartLoading(const QUrl& url);
  void TrackSkipped();
  LoadResult LoadNext(const QUrl& url);

private:
  VkService* service_;
};

#endif  // VKURLHANDLER_H
