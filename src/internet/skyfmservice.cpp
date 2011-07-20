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

#include "digitallyimportedurlhandler.h"
#include "internetmodel.h"
#include "skyfmservice.h"
#include "core/taskmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

SkyFmService::SkyFmService(InternetModel* model, QObject* parent)
  : DigitallyImportedServiceBase(
      "SKY.fm", "SKY.fm", QUrl("http://www.sky.fm"), "sky.fm",
      QUrl("http://listen.sky.fm"), "skyfm", QIcon(":/providers/skyfm.png"),
      model, parent)
{
  playlists_ = QList<Playlist>()
    << Playlist(false, "http://listen.sky.fm/public3/%1.pls")
    << Playlist(true,  "http://listen.sky.fm/premium_high/%1.pls?hash=%2")
    << Playlist(false, "http://listen.sky.fm/public1/%1.pls")
    << Playlist(true,  "http://listen.sky.fm/premium_medium/%1.pls?hash=%2")
    << Playlist(true,  "http://listen.sky.fm/premium/%1.pls?hash=%2")
    << Playlist(false, "http://listen.sky.fm/public5/%1.asx")
    << Playlist(true,  "http://listen.sky.fm/premium_wma_low/%1.asx?hash=%2")
    << Playlist(true,  "http://listen.sky.fm/premium_wma/%1.asx?hash=%2");
}

void SkyFmService::LoadStation(const QString& key) {
  if (!is_premium_stream_selected()) {
    // Non-premium streams can just start loading straight away
    LoadPlaylist(key);
    return;
  }

  // Otherwise we have to get the user's hashKey
  QNetworkRequest req(QUrl("http://www.sky.fm/configure_player.php"));
  QByteArray postdata = "amember_login=" + QUrl::toPercentEncoding(username_) +
                        "&amember_pass=" + QUrl::toPercentEncoding(password_);

  QNetworkReply* reply = network_->post(req, postdata);
  connect(reply, SIGNAL(finished()), SLOT(LoadHashKeyFinished()));

  last_key_ = key;
}

void SkyFmService::LoadHashKeyFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    return;
  }

  const QString page_data = QString::fromUtf8(reply->readAll().data());
  QRegExp re("hashKey\\s*=\\s*'([0-9a-f]+)'");

  if (re.indexIn(page_data) == -1) {
    url_handler_->CancelTask();
    emit StreamError(tr("Invalid SKY.fm username or password"));
    return;
  }

  LoadPlaylist(last_key_, re.cap(1));
}

void SkyFmService::LoadPlaylist(const QString& key, const QString& hash_key) {
  QString url_template = playlists_[audio_type_].url_template_;
  QUrl url;

  if (hash_key.isEmpty()) {
    url = QUrl(url_template.arg(key));
  } else {
    url = QUrl(url_template.arg(key, hash_key));
  }

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));
}
