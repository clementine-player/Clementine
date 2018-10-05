/* This file is part of Clementine.
   Copyright 2009-2013, David Sansome <me@davidsansome.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2012, Kacper "mattrick" Banasik <mattrick@jabster.pl>
   Copyright 2012, Harald Sitter <sitter@kde.org>
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

// StringBuilder is activated to speed-up QString concatenation. As explained
// here:
// http://labs.qt.nokia.com/2011/06/13/string-concatenation-with-qstringbuilder/
// this cause some compilation errors in some cases. As Lasfm library inlines
// some functions in their includes files, which aren't compatible with
// QStringBuilder, we undef it here
#include <QtGlobal>
#if QT_VERSION >= 0x040600
#if QT_VERSION >= 0x040800
#undef QT_USE_QSTRINGBUILDER
#else
#undef QT_USE_FAST_CONCATENATION
#undef QT_USE_FAST_OPERATOR_PLUS
#endif  // QT_VERSION >= 0x040800
#endif  // QT_VERSION >= 0x040600

#include "lastfmservice.h"

#include <algorithm>

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>

#ifdef HAVE_LIBLASTFM1
#include <lastfm/RadioStation.h>
#else
#include <lastfm/RadioStation>
#endif

#include "lastfmcompat.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "internet/core/internetmodel.h"
#include "internet/core/internetplaylistitem.h"
#include "internet/core/localredirectserver.h"
#include "covers/coverproviders.h"
#include "covers/lastfmcoverprovider.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"

using lastfm::XmlQuery;

uint qHash(const lastfm::Track& track) {
  return qHash(track.title()) ^ qHash(track.artist().name()) ^
         qHash(track.album().title());
}

const char* LastFMService::kServiceName = "Last.fm";
const char* LastFMService::kSettingsGroup = "Last.fm";
const char* LastFMService::kAudioscrobblerClientId = "tng";
const char* LastFMService::kApiKey = "75d20fb472be99275392aefa2760ea09";
const char* LastFMService::kSecret = "d3072b60ae626be12be69448f5c46e70";
const char* LastFMService::kAuthLoginUrl =
    "https://www.last.fm/api/auth/?api_key=%1&token=%2";

LastFMService::LastFMService(Application* app, QObject* parent)
    : Scrobbler(parent),
      already_scrobbled_(false),
      scrobbling_enabled_(false),
      connection_problems_(false),
      app_(app),
      network_(new NetworkAccessManager) {
#ifdef HAVE_LIBLASTFM1
  lastfm::ws::setScheme(lastfm::ws::Https);
#endif

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
  lastfm::ws::Username = settings.value("Username").toString();
  lastfm::ws::SessionKey = settings.value("Session").toString();
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
  return !lastfm::ws::SessionKey.isEmpty();
}

bool LastFMService::IsSubscriber() const {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  return settings.value("Subscriber", false).toBool();
}

namespace {
QByteArray SignApiRequest(QList<QPair<QString, QString>> params) {
  std::sort(params.begin(), params.end());
  QString to_sign;
  for (const auto& p : params) {
    to_sign += p.first;
    to_sign += p.second;
  }
  to_sign += LastFMService::kSecret;
  return QCryptographicHash::hash(to_sign.toUtf8(), QCryptographicHash::Md5).toHex();
}
}  // namespace

void LastFMService::Authenticate() {
  QUrl url("https://www.last.fm/api/auth/");
  url.addQueryItem("api_key", kApiKey);

  LocalRedirectServer* server = new LocalRedirectServer(this);
  server->Listen();

  url.addQueryItem("cb", server->url().toString());

  NewClosure(server, SIGNAL(Finished()), [this, server]() {
    server->deleteLater();

    const QUrl& url = server->request_url();
    QString token = url.queryItemValue("token");

    QUrl session_url("https://ws.audioscrobbler.com/2.0/");
    session_url.addQueryItem("api_key", kApiKey);
    session_url.addQueryItem("method", "auth.getSession");
    session_url.addQueryItem("token", token);
    session_url.addQueryItem("api_sig", SignApiRequest(session_url.queryItems()));

    QNetworkReply* reply = network_->get(QNetworkRequest(session_url));
    NewClosure(reply, SIGNAL(finished()), this, SLOT(AuthenticateReplyFinished(QNetworkReply*)), reply);
  });

  if (!QDesktopServices::openUrl(url)) {
    QMessageBox box(QMessageBox::NoIcon, tr("Last.fm Authentication"), tr("Please open this URL in your browser: <a href=\"%1\">%1</a>").arg(url.toString()), QMessageBox::Ok);
    box.setTextFormat(Qt::RichText);
    qLog(Debug) << "Last.fm authentication URL: " << url.toString();
    box.exec();
  }
}

void LastFMService::AuthenticateReplyFinished(QNetworkReply* reply) {
  reply->deleteLater();

  // Parse the reply
  lastfm::XmlQuery lfm(lastfm::compat::EmptyXmlQuery());
  if (lastfm::compat::ParseQuery(reply->readAll(), &lfm)) {
    lastfm::ws::Username = lfm["session"]["name"].text();
    lastfm::ws::SessionKey = lfm["session"]["key"].text();
    QString subscribed = lfm["session"]["subscriber"].text();
    const bool is_subscriber = (subscribed.toInt() == 1);

    // Save the session key
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    settings.setValue("Username", lastfm::ws::Username);
    settings.setValue("Session", lastfm::ws::SessionKey);
    settings.setValue("Subscriber", is_subscriber);
  } else {
    emit AuthenticationComplete(false);
    return;
  }

  // Invalidate the scrobbler - it will get recreated later
  scrobbler_.reset(nullptr);

  emit AuthenticationComplete(true);
}

void LastFMService::SignOut() {
  lastfm::ws::Username.clear();
  lastfm::ws::SessionKey.clear();

  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  settings.setValue("Username", QString());
  settings.setValue("Session", QString());
}

void LastFMService::UpdateSubscriberStatus() {
  QMap<QString, QString> params;
  params["method"] = "user.getInfo";
  params["user"] = lastfm::ws::Username;

  QNetworkReply* reply = lastfm::ws::post(params);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(UpdateSubscriberStatusFinished(QNetworkReply*)), reply);
}

void LastFMService::UpdateSubscriberStatusFinished(QNetworkReply* reply) {
  reply->deleteLater();

  bool is_subscriber = false;

  lastfm::XmlQuery lfm(lastfm::compat::EmptyXmlQuery());
  if (lastfm::compat::ParseQuery(reply->readAll(), &lfm,
                                 &connection_problems_)) {
    QString subscriber = lfm["user"]["subscriber"].text();
    is_subscriber = (subscriber.toInt() == 1);

    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    settings.setValue("Subscriber", is_subscriber);
    qLog(Info) << lastfm::ws::Username << "Subscriber status:" << is_subscriber;
  }

  emit UpdatedSubscriberStatus(is_subscriber);
}

QUrl LastFMService::FixupUrl(const QUrl& url) {
  QUrl ret;
  ret.setEncodedUrl(url.toEncoded().replace(
      "USERNAME", QUrl::toPercentEncoding(lastfm::ws::Username)));
  return ret;
}

QString LastFMService::ErrorString(lastfm::ws::Error error) const {
  switch (error) {
    case lastfm::ws::InvalidService:
      return tr("Invalid service");
    case lastfm::ws::InvalidMethod:
      return tr("Invalid method");
    case lastfm::ws::AuthenticationFailed:
      return tr("Authentication failed");
    case lastfm::ws::InvalidFormat:
      return tr("Invalid format");
    case lastfm::ws::InvalidParameters:
      return tr("Invalid parameters");
    case lastfm::ws::InvalidResourceSpecified:
      return tr("Invalid resource specified");
    case lastfm::ws::OperationFailed:
      return tr("Operation failed");
    case lastfm::ws::InvalidSessionKey:
      return tr("Invalid session key");
    case lastfm::ws::InvalidApiKey:
      return tr("Invalid API key");
    case lastfm::ws::ServiceOffline:
      return tr("Service offline");
    case lastfm::ws::SubscribersOnly:
      return tr("This stream is for paid subscribers only");

    case lastfm::ws::TryAgainLater:
      return tr("Last.fm is currently busy, please try again in a few minutes");

    case lastfm::ws::NotEnoughContent:
      return tr("Not enough content");
    case lastfm::ws::NotEnoughMembers:
      return tr("Not enough members");
    case lastfm::ws::NotEnoughFans:
      return tr("Not enough fans");
    case lastfm::ws::NotEnoughNeighbours:
      return tr("Not enough neighbors");

    case lastfm::ws::MalformedResponse:
      return tr("Malformed response");

    case lastfm::ws::UnknownError:
    default:
      return tr("Unknown error");
  }
}

bool LastFMService::InitScrobbler() {
  if (!IsAuthenticated() || !IsScrobblingEnabled()) return false;

  if (!scrobbler_)
    scrobbler_.reset(new lastfm::Audioscrobbler(kAudioscrobblerClientId));

// reemit the signal since the sender is private
#ifdef HAVE_LIBLASTFM1
  connect(scrobbler_.get(), SIGNAL(scrobblesSubmitted(QList<lastfm::Track>)),
          SIGNAL(ScrobbleSubmitted()));
  connect(scrobbler_.get(), SIGNAL(nowPlayingError(int, QString)),
          SIGNAL(ScrobbleError(int)));
#else
  connect(scrobbler_.get(), SIGNAL(status(int)), SLOT(ScrobblerStatus(int)));
#endif
  return true;
}

void LastFMService::ScrobblerStatus(int value) {
  switch (value) {
    case 2:
    case 3:
      emit ScrobbleSubmitted();
      break;

    default:
      emit ScrobbleError(value);
      break;
  }
}

lastfm::Track LastFMService::TrackFromSong(const Song& song) const {
  if (song.title() == last_track_.title() &&
      song.artist() == last_track_.artist() &&
      song.album() == last_track_.album())
    return last_track_;

  lastfm::Track ret;
  song.ToLastFM(&ret, PreferAlbumArtist());
  return ret;
}

void LastFMService::NowPlaying(const Song& song) {
  if (!InitScrobbler()) return;

  // Scrobbling streams is difficult if we don't have length of each individual
  // part.  In Song::ToLastFm we set the Track's source to
  // NonPersonalisedBroadcast if it's such a stream, so we have to scrobble it
  // when we change to a different track, but only if enough time has elapsed
  // since it started playing.
  if (!last_track_.isNull() &&
      last_track_.source() == lastfm::Track::NonPersonalisedBroadcast) {
    const int duration_secs =
        last_track_.timestamp().secsTo(QDateTime::currentDateTime());
    if (duration_secs >= lastfm::compat::ScrobbleTimeMin()) {
      lastfm::MutableTrack mtrack(last_track_);
      mtrack.setDuration(duration_secs);

      qLog(Info) << "Scrobbling stream track" << mtrack.title() << "length"
                 << duration_secs;
      scrobbler_->cache(mtrack);
      scrobbler_->submit();

      emit ScrobbledRadioStream();
    }
  }

  lastfm::MutableTrack mtrack(TrackFromSong(song));
  mtrack.stamp();
  already_scrobbled_ = false;
  last_track_ = mtrack;

#ifndef HAVE_LIBLASTFM1
  // Check immediately if the song is valid
  Scrobble::Invalidity invalidity;
  if (!lastfm::Scrobble(last_track_).isValid(&invalidity)) {
    // for now just notify this, we can also see the cause
    emit ScrobbleError(-1);
    return;
  }
#else
// TODO(John Maguire): validity was removed from liblastfm1 but might reappear,
// it should have
// no impact as we get a different error when actually trying to scrobble.
#endif

  scrobbler_->nowPlaying(mtrack);
}

void LastFMService::Scrobble() {
  if (!InitScrobbler()) return;

  lastfm::compat::ScrobbleCache cache(lastfm::ws::Username);
  qLog(Debug) << "There are" << cache.tracks().count()
              << "tracks in the last.fm cache.";
  scrobbler_->cache(last_track_);

  // Let's mark a track as cached, useful when the connection is down
  emit ScrobbleError(30);
  scrobbler_->submit();

  already_scrobbled_ = true;
}

void LastFMService::Love() {
  if (!IsAuthenticated()) ShowConfig();

  lastfm::MutableTrack mtrack(last_track_);
  mtrack.love();
  last_track_ = mtrack;

  if (already_scrobbled_) {
    // The love only takes effect when the song is scrobbled, but we've already
    // scrobbled this one so we have to do it again.
    Scrobble();
  }
}

void LastFMService::Ban() {
  if (!IsAuthenticated()) ShowConfig();

  lastfm::MutableTrack mtrack(last_track_);
  mtrack.ban();
  last_track_ = mtrack;

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
