/* This file is part of Clementine.
   Copyright 2009-2013, David Sansome <me@davidsansome.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2012, Kacper "mattrick" Banasik <mattrick@jabster.pl>
   Copyright 2012, Harald Sitter <sitter@kde.org>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
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

#include "lastfmservice.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSettings>
#include <QUrlQuery>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/song.h"
#include "internet/core/localredirectserver.h"
#include "lastfmscrobblecache.h"
#include "lastfmws.h"
#include "lastfmxmlquery.h"
#include "ui/settingsdialog.h"

namespace {
// Last.fm won't accept a scrobble for a track shorter than this.
const int kScrobbleTimeMinSecs = 31;
}  // namespace

const char* LastFMService::kServiceName = "Last.fm";
const char* LastFMService::kSettingsGroup = "Last.fm";
const char* LastFMService::kApiKey = "75d20fb472be99275392aefa2760ea09";
const char* LastFMService::kSecret = "d3072b60ae626be12be69448f5c46e70";

LastFMService::LastFMService(Application* app, QObject* parent)
    : Scrobbler(parent),
      scrobbling_enabled_(false),
      connection_problems_(false),
      app_(app),
      network_(new NetworkAccessManager) {
  ReloadSettings();

  // we emit the signal the first time to be sure the buttons are in the right
  // state
  emit ScrobblingEnabledChanged(scrobbling_enabled_);
}

LastFMService::~LastFMService() {}

void LastFMService::ReloadSettings() {
  bool scrobbling_enabled_old = scrobbling_enabled_;
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  LastFmWs::username = settings.value("Username").toString();
  LastFmWs::session_key = settings.value("Session").toString();
  scrobbling_enabled_ = settings.value("ScrobblingEnabled", true).toBool();
  buttons_visible_ = settings.value("ShowLoveBanButtons", true).toBool();
  scrobble_button_visible_ =
      settings.value("ShowScrobbleButton", true).toBool();
  prefer_albumartist_ = settings.value("PreferAlbumArtist", false).toBool();

  // avoid emitting signal if it's not changed
  if (scrobbling_enabled_old != scrobbling_enabled_)
    emit ScrobblingEnabledChanged(scrobbling_enabled_);
  emit ButtonVisibilityChanged(buttons_visible_);
  emit ScrobbleButtonVisibilityChanged(scrobble_button_visible_);
  emit PreferAlbumArtistChanged(prefer_albumartist_);
}

void LastFMService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Lastfm);
}

bool LastFMService::IsAuthenticated() const {
  return !LastFmWs::session_key.isEmpty();
}

bool LastFMService::IsSubscriber() const {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  return settings.value("Subscriber", false).toBool();
}

void LastFMService::Authenticate() {
  QUrl url("https://www.last.fm/api/auth/");

  LocalRedirectServer* server = new LocalRedirectServer(this);
  server->Listen();

  QUrlQuery url_query;
  url_query.addQueryItem("api_key", kApiKey);
  url_query.addQueryItem("cb", server->url().toString());
  url.setQuery(url_query);

  NewClosure(server, SIGNAL(Finished()), [this, server]() {
    server->deleteLater();

    const QUrl& url = server->request_url();
    QString token = QUrlQuery(url).queryItemValue("token");

    QMap<QString, QString> params;
    params["api_key"] = kApiKey;
    params["method"] = "auth.getSession";
    params["token"] = token;
    params["api_sig"] = LastFmWs::Sign(params);

    QUrl session_url("https://ws.audioscrobbler.com/2.0/");
    QUrlQuery session_url_query;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
      session_url_query.addQueryItem(it.key(), it.value());
    }
    session_url.setQuery(session_url_query);

    QNetworkReply* reply = network_->get(QNetworkRequest(session_url));
    NewClosure(reply, SIGNAL(finished()), this,
               SLOT(AuthenticateReplyFinished(QNetworkReply*)), reply);
  });

  if (!QDesktopServices::openUrl(url)) {
    QMessageBox box(
        QMessageBox::NoIcon, tr("Last.fm Authentication"),
        tr("Please open this URL in your browser: <a href=\"%1\">%1</a>")
            .arg(url.toString()),
        QMessageBox::Ok);
    box.setTextFormat(Qt::RichText);
    qLog(Debug) << "Last.fm authentication URL: " << url.toString();
    box.exec();
  }
}

void LastFMService::AuthenticateReplyFinished(QNetworkReply* reply) {
  reply->deleteLater();

  LastFmXmlQuery lfm;
  if (lfm.Parse(reply->readAll())) {
    LastFmWs::username = lfm["session"]["name"].text();
    LastFmWs::session_key = lfm["session"]["key"].text();
    QString subscribed = lfm["session"]["subscriber"].text();
    const bool is_subscriber = (subscribed.toInt() == 1);

    // Save the session key
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    settings.setValue("Username", LastFmWs::username);
    settings.setValue("Session", LastFmWs::session_key);
    settings.setValue("Subscriber", is_subscriber);
  } else {
    emit AuthenticationComplete(false);
    return;
  }

  emit AuthenticationComplete(true);
}

void LastFMService::SignOut() {
  LastFmWs::username.clear();
  LastFmWs::session_key.clear();

  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  settings.setValue("Username", QString());
  settings.setValue("Session", QString());
}

LastFmTrack LastFMService::TrackFromSong(const Song& song) const {
  if (song.title() == last_track_.title && song.artist() == last_track_.artist &&
      song.album() == last_track_.album)
    return last_track_;

  return song.ToLastFmTrack(PreferAlbumArtist());
}

void LastFMService::NowPlaying(const Song& song) {
  if (!IsAuthenticated() || !IsScrobblingEnabled()) return;

  // Scrobbling streams is difficult if we don't have the length of each
  // individual part. Song::ToLastFmTrack sets the track's source to
  // NonPersonalisedBroadcast if it's such a stream, so we have to scrobble it
  // when we change to a different track, but only if enough time has elapsed
  // since it started playing.
  if (last_track_.is_valid() &&
      last_track_.source == LastFmTrack::NonPersonalisedBroadcast) {
    const int duration_secs =
        last_track_.timestamp.secsTo(QDateTime::currentDateTime());
    if (duration_secs >= kScrobbleTimeMinSecs) {
      LastFmTrack finished_track = last_track_;
      finished_track.duration_secs = duration_secs;

      qLog(Info) << "Scrobbling stream track" << finished_track.title
                 << "length" << duration_secs;
      LastFmScrobbleCache(LastFmWs::username).Add(finished_track);
      Scrobble();

      emit ScrobbledRadioStream();
    }
  }

  LastFmTrack track = TrackFromSong(song);
  track.timestamp = QDateTime::currentDateTime();
  already_cached_to_scrobble_ = false;
  last_track_ = track;

  if (!track.is_valid()) {
    emit ScrobbleError(-1);
    return;
  }

  QMap<QString, QString> params;
  params["method"] = "track.updateNowPlaying";
  params["artist"] = track.artist;
  params["track"] = track.title;
  if (!track.album.isEmpty()) params["album"] = track.album;
  if (!track.album_artist.isEmpty())
    params["albumArtist"] = track.album_artist;
  if (track.track_number > 0)
    params["trackNumber"] = QString::number(track.track_number);
  if (track.duration_secs > 0)
    params["duration"] = QString::number(track.duration_secs);

  QNetworkReply* reply = LastFmWs::Post(params);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(NowPlayingReplyFinished(QNetworkReply*)), reply);
}

void LastFMService::NowPlayingReplyFinished(QNetworkReply* reply) {
  reply->deleteLater();
  LastFmXmlQuery lfm;
  if (!lfm.Parse(reply->readAll())) {
    emit ScrobbleError(-1);
  }
}

void LastFMService::CacheSong(int scrobble_point) {
  if (!IsAuthenticated() || !IsScrobblingEnabled()) return;

  if (!already_cached_to_scrobble_ && scrobble_point) {
    qLog(Info) << "Caching song to scrobble at" << scrobble_point;
    LastFmScrobbleCache(LastFmWs::username).Add(last_track_);
    already_cached_to_scrobble_ = true;
  }
  emit CachedToScrobble();
}

void LastFMService::Scrobble() {
  if (!IsAuthenticated() || !IsScrobblingEnabled()) return;

  LastFmScrobbleCache cache(LastFmWs::username);
  qLog(Debug) << "There are" << cache.tracks().count()
              << "tracks in the last.fm cache before submit request.";

  // Let's mark a track as cached, useful when the connection is down
  emit ScrobbleError(30);
  if (cache.isEmpty()) return;

  // Last.fm allows submitting up to 50 tracks in a single request.
  const QList<LastFmTrack> tracks = cache.tracks();
  const int batch_size = qMin(tracks.count(), 50);

  QMap<QString, QString> params;
  params["method"] = "track.scrobble";
  for (int i = 0; i < batch_size; ++i) {
    const LastFmTrack& track = tracks.at(i);
    const QString suffix = QString("[%1]").arg(i);
    params["artist" + suffix] = track.artist;
    params["track" + suffix] = track.title;
    params["timestamp" + suffix] =
        QString::number(track.timestamp.toSecsSinceEpoch());
    if (!track.album.isEmpty()) params["album" + suffix] = track.album;
    if (!track.album_artist.isEmpty())
      params["albumArtist" + suffix] = track.album_artist;
    if (track.track_number > 0)
      params["trackNumber" + suffix] = QString::number(track.track_number);
    if (track.duration_secs > 0)
      params["duration" + suffix] = QString::number(track.duration_secs);
    params["chosenByUser" + suffix] =
        track.source == LastFmTrack::Player ? "1" : "0";
  }

  QNetworkReply* reply = LastFmWs::Post(params);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(ScrobbleReplyFinished(QNetworkReply*, int)), reply,
             batch_size);
}

void LastFMService::ScrobbleReplyFinished(QNetworkReply* reply,
                                          int submitted_count) {
  reply->deleteLater();

  LastFmXmlQuery lfm;
  if (lfm.Parse(reply->readAll())) {
    LastFmScrobbleCache(LastFmWs::username).RemoveFirst(submitted_count);
    emit ScrobbleSubmitted();
  } else {
    emit ScrobbleError(-1);
  }
}

void LastFMService::Love() {
  if (!IsAuthenticated()) ShowConfig();

  QMap<QString, QString> params;
  params["method"] = "track.love";
  params["artist"] = last_track_.artist;
  params["track"] = last_track_.title;
  QNetworkReply* reply = LastFmWs::Post(params);
  connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void LastFMService::Ban() {
  if (!IsAuthenticated()) ShowConfig();

  QMap<QString, QString> params;
  params["method"] = "track.ban";
  params["artist"] = last_track_.artist;
  params["track"] = last_track_.title;
  QNetworkReply* reply = LastFmWs::Post(params);
  connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));

  CacheSong(0);
  Scrobble();
  app_->player()->Next();
}

void LastFMService::ToggleScrobbling() {
  // toggle status
  scrobbling_enabled_ = !scrobbling_enabled_;

  // save to the settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("ScrobblingEnabled", scrobbling_enabled_);
  s.endGroup();

  emit ScrobblingEnabledChanged(scrobbling_enabled_);
}
