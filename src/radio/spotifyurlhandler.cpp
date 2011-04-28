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

#include "spotifyserver.h"
#include "spotifyservice.h"
#include "spotifyurlhandler.h"
#include "core/logging.h"

#include <QTcpServer>

SpotifyUrlHandler::SpotifyUrlHandler(SpotifyService* service, QObject* parent)
  : UrlHandler(parent),
    service_(service) {
}

UrlHandler::LoadResult SpotifyUrlHandler::StartLoading(const QUrl& url) {
  // Pick an unused local port.  There's a possible race condition here -
  // something else might grab the port before gstreamer does.
  quint16 port = 0;

  {
    QTcpServer server;
    server.listen(QHostAddress::LocalHost);
    port = server.serverPort();
  }

  if (port == 0) {
    qLog(Warning) << "Couldn't pick an unused port";
    return LoadResult();
  }

  // Tell Spotify to start sending to this port
  service_->server()->StartPlayback(url.toString(), port);

  // Tell gstreamer to listen on this port
  return LoadResult(url, LoadResult::TrackAvailable,
                    QUrl("tcp://localhost:" + QString::number(port)));
}
