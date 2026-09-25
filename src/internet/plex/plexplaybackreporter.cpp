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

#include "plexplaybackreporter.h"

#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "engines/enginebase.h"
#include "internet/plex/plexparser.h"
#include "internet/plex/plexservice.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"

namespace {

const int kTimelineIntervalMsec = 10000;
const char* kLibraryIdentifier = "com.plexapp.plugins.library";

}  // namespace

PlexPlaybackReporter::PlexPlaybackReporter(PlexService* service,
                                           Application* app, QObject* parent)
    : QObject(parent),
      service_(service),
      app_(app),
      timer_(new QTimer(this)),
      time_msec_(0),
      duration_msec_(0),
      scrobbled_(false) {
  timer_->setInterval(kTimelineIntervalMsec);
  connect(timer_, SIGNAL(timeout()), SLOT(Tick()));

  connect(app_->player(), SIGNAL(Playing()), SLOT(Playing()));
  connect(app_->player(), SIGNAL(Paused()), SLOT(Paused()));
  connect(app_->player(), SIGNAL(Stopped()), SLOT(Stopped()));
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(CurrentSongChanged(Song)));
}

void PlexPlaybackReporter::Playing() { Update("playing"); }

void PlexPlaybackReporter::Paused() { Update("paused"); }

void PlexPlaybackReporter::Tick() { Update("playing"); }

void PlexPlaybackReporter::CurrentSongChanged(const Song&) {
  const Engine::State state = app_->player()->GetState();
  if (state == Engine::Playing) {
    Update("playing");
  } else if (state == Engine::Paused) {
    Update("paused");
  }
}

void PlexPlaybackReporter::Stopped() {
  timer_->stop();
  if (rating_key_.isEmpty()) return;
  SendTimeline("stopped");
  rating_key_.clear();
}

QString PlexPlaybackReporter::CurrentRatingKey() const {
  if (service_->login_state() != PlexService::LoginState_LoggedIn) {
    return QString();
  }

  PlaylistItemPtr item = app_->player()->GetCurrentItem();
  if (!item) return QString();

  const QUrl url = item->Url();
  if (url.scheme() != PlexParser::kScheme ||
      PlexParser::MachineIdFromUrl(url) != service_->machine_id()) {
    return QString();
  }
  return PlexParser::RatingKeyFromUrl(url);
}

void PlexPlaybackReporter::Update(const QString& state) {
  const QString key = CurrentRatingKey();
  if (key != rating_key_) {
    // The track changed without a stop, e.g. moving on to the next one.
    if (!rating_key_.isEmpty()) SendTimeline("stopped");
    rating_key_ = key;
    scrobbled_ = false;
  }

  if (rating_key_.isEmpty()) {
    timer_->stop();
    return;
  }

  const qint64 position = app_->player()->engine()->position_nanosec();
  time_msec_ = position / kNsecPerMsec;
  duration_msec_ = app_->player()->engine()->length_nanosec() / kNsecPerMsec;

  MaybeScrobble(position);
  SendTimeline(state);

  if (state == "playing") {
    if (!timer_->isActive()) timer_->start();
  } else {
    timer_->stop();
  }
}

void PlexPlaybackReporter::SendTimeline(const QString& state) {
  QUrlQuery query;
  query.addQueryItem("ratingKey", rating_key_);
  query.addQueryItem("key", "/library/metadata/" + rating_key_);
  query.addQueryItem("state", state);
  query.addQueryItem("time", QString::number(time_msec_));
  query.addQueryItem("duration", QString::number(duration_msec_));

  QNetworkReply* reply = service_->Get("/:/timeline", query);
  connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void PlexPlaybackReporter::MaybeScrobble(qint64 position_nanosec) {
  if (scrobbled_) return;

  // Same rule as the local library's play counts.
  Playlist* playlist = app_->playlist_manager()->active();
  if (position_nanosec < playlist->play_count_point_nanosec() ||
      playlist->get_lastfm_status() == Playlist::LastFM_Seeked) {
    return;
  }

  scrobbled_ = true;
  service_->NoteOwnChange(rating_key_);

  QUrlQuery query;
  query.addQueryItem("identifier", kLibraryIdentifier);
  query.addQueryItem("key", rating_key_);
  QNetworkReply* reply = service_->Get("/:/scrobble", query);
  connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
  qLog(Debug) << "Scrobbled Plex track" << rating_key_;
}
