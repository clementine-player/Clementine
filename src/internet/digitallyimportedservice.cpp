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

#include "digitallyimportedservice.h"
#include "core/logging.h"

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>

DigitallyImportedService::DigitallyImportedService(InternetModel* model, QObject* parent)
  : DigitallyImportedServiceBase(
      "DigitallyImported", "Digitally Imported", QUrl("http://www.di.fm"),
      "di.fm", QUrl("http://listen.di.fm"), "digitallyimported",
      QIcon(":/providers/digitallyimported.png"), model, parent)
{
  playlists_ = QList<Playlist>()
    << Playlist(false, "http://listen.di.fm/public3/%1.pls")
    << Playlist(true,  "http://www.di.fm/listen/%1/premium.pls")
    << Playlist(false, "http://listen.di.fm/public2/%1.pls")
    << Playlist(true,  "http://www.di.fm/listen/%1/64k.pls")
    << Playlist(true,  "http://www.di.fm/listen/%1/128k.pls")
    << Playlist(false, "http://listen.di.fm/public5/%1.asx")
    << Playlist(true,  "http://www.di.fm/listen/%1/64k.asx")
    << Playlist(true,  "http://www.di.fm/listen/%1/128k.asx");
}

void DigitallyImportedService::ReloadSettings() {
  DigitallyImportedServiceBase::ReloadSettings();

  QNetworkCookieJar* cookies = new QNetworkCookieJar;

  if (is_premium_account()) {
    qLog(Debug) << "Setting premium account cookies";
    cookies->setCookiesFromUrl(QList<QNetworkCookie>()
        << QNetworkCookie("_amember_ru", username_.toUtf8())
        << QNetworkCookie("_amember_rp", password_.toUtf8()),
                               QUrl("http://www.di.fm/"));
  }

  network_->setCookieJar(cookies);
}

void DigitallyImportedService::LoadStation(const QString& key) {
  QUrl playlist_url(playlists_[audio_type_].url_template_.arg(key));
  qLog(Debug) << "Getting playlist URL" << playlist_url;

  QNetworkReply* reply = network_->get(QNetworkRequest(playlist_url));
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));
}
