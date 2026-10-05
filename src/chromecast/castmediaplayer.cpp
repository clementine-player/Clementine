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

#include "castmediaplayer.h"

#include <QJsonArray>

#include "castchannel.h"
#include "core/logging.h"

const char* CastMediaPlayer::kDefaultMediaReceiverAppId = "CC1AD845";
const char* CastMediaPlayer::kReceiverNamespace =
    "urn:x-cast:com.google.cast.receiver";
const char* CastMediaPlayer::kMediaNamespace =
    "urn:x-cast:com.google.cast.media";

namespace {

// From the Cast media messages' MetadataType.
const int kMusicTrackMetadata = 3;

}  // namespace

CastMediaPlayer::CastMediaPlayer(CastChannel* channel, QObject* parent)
    : QObject(parent),
      channel_(channel),
      next_request_id_(1),
      launching_(false),
      media_session_id_(0),
      load_request_id_(0),
      idle_media_session_id_(0),
      state_(Idle),
      position_ms_(0),
      playback_rate_(1.0),
      duration_ms_(0),
      volume_(1.0),
      muted_(false) {
  connect(channel_, SIGNAL(MessageReceived(QString, QString, QJsonObject)),
          SLOT(MessageReceived(QString, QString, QJsonObject)));
}

void CastMediaPlayer::Launch() {
  if (is_ready()) {
    emit Ready();
    return;
  }
  launching_ = true;
  SendReceiver({{"type", "LAUNCH"}, {"appId", kDefaultMediaReceiverAppId}});
}

void CastMediaPlayer::StopApp() {
  if (session_id_.isEmpty()) return;
  SendReceiver({{"type", "STOP"}, {"sessionId", session_id_}});
  AppGone();
}

void CastMediaPlayer::Load(const CastMediaInfo& info, qint64 start_ms,
                           bool autoplay) {
  if (!is_ready()) {
    qLog(Warning) << "Can't load a track before the Cast media app is running";
    return;
  }

  QJsonObject metadata{{"metadataType", kMusicTrackMetadata}};
  if (!info.title.isEmpty()) metadata["title"] = info.title;
  if (!info.artist.isEmpty()) metadata["artist"] = info.artist;
  if (!info.album.isEmpty()) metadata["albumName"] = info.album;
  if (info.image_url.isValid()) {
    metadata["images"] =
        QJsonArray{QJsonObject{{"url", info.image_url.toString()}}};
  }

  QJsonObject media{{"contentId", info.url.toString()},
                    {"contentType", info.content_type},
                    {"streamType", "BUFFERED"},
                    {"metadata", metadata}};
  if (info.duration_ms > 0) media["duration"] = info.duration_ms / 1000.0;

  // The new track's status replaces the old one's.
  media_session_id_ = 0;
  duration_ms_ = info.duration_ms;
  position_ms_ = start_ms;
  position_updated_.start();
  load_request_id_ = next_request_id_;
  SendMedia({{"type", "LOAD"},
             {"media", media},
             {"autoplay", autoplay},
             {"currentTime", start_ms / 1000.0}});
  SetState(Buffering);
}

void CastMediaPlayer::Play() { SendMediaCommand("PLAY"); }

void CastMediaPlayer::Pause() { SendMediaCommand("PAUSE"); }

void CastMediaPlayer::Stop() { SendMediaCommand("STOP"); }

void CastMediaPlayer::Seek(qint64 position_ms) {
  SendMediaCommand("SEEK", {{"currentTime", position_ms / 1000.0}});
  position_ms_ = position_ms;
  position_updated_.start();
}

void CastMediaPlayer::SetVolume(double level) {
  SendReceiver({{"type", "SET_VOLUME"},
                {"volume", QJsonObject{{"level", qBound(0.0, level, 1.0)}}}});
}

void CastMediaPlayer::SetMuted(bool muted) {
  SendReceiver(
      {{"type", "SET_VOLUME"}, {"volume", QJsonObject{{"muted", muted}}}});
}

qint64 CastMediaPlayer::position_ms() const {
  if (state_ != Playing || !position_updated_.isValid()) return position_ms_;
  qint64 position =
      position_ms_ + qint64(position_updated_.elapsed() * playback_rate_);
  if (duration_ms_ > 0) position = qMin(position, duration_ms_);
  return position;
}

void CastMediaPlayer::SendReceiver(QJsonObject payload) {
  payload["requestId"] = next_request_id_++;
  channel_->Send(kReceiverNamespace, CastChannel::kReceiverId, payload);
}

void CastMediaPlayer::SendMedia(QJsonObject payload) {
  if (!is_ready()) {
    qLog(Warning) << "The Cast media app isn't running";
    return;
  }
  payload["requestId"] = next_request_id_++;
  channel_->Send(kMediaNamespace, transport_id_, payload);
}

void CastMediaPlayer::SendMediaCommand(const QString& type,
                                       QJsonObject payload) {
  if (media_session_id_ == 0) {
    qLog(Debug) << "Nothing loaded on the Cast device for" << type;
    return;
  }
  payload["type"] = type;
  payload["mediaSessionId"] = media_session_id_;
  SendMedia(payload);
}

void CastMediaPlayer::MessageReceived(const QString& name_space,
                                      const QString& source_id,
                                      const QJsonObject& payload) {
  const QString type = payload["type"].toString();

  if (name_space == kReceiverNamespace) {
    if (type == "RECEIVER_STATUS") {
      ReceiverStatus(payload["status"].toObject());
    } else if (type == "LAUNCH_ERROR") {
      launching_ = false;
      emit Error(tr("The Cast device couldn't start playing: %1")
                     .arg(payload["reason"].toString()));
    }
    return;
  }

  if (name_space == kMediaNamespace && source_id == transport_id_) {
    if (type == "MEDIA_STATUS") {
      const QJsonArray statuses = payload["status"].toArray();
      // Only the LOAD's own status names the new media session, so older
      // ones that arrive first are ignored.
      if (media_session_id_ == 0 && load_request_id_ != 0 &&
          payload["requestId"].toInt() != load_request_id_) {
        return;
      }
      if (statuses.isEmpty()) {
        // Nothing loaded any more.
        if (media_session_id_ != 0) {
          media_session_id_ = 0;
          SetState(Idle);
        }
        return;
      }
      MediaStatus(statuses[0].toObject());
    } else if (type == "LOAD_FAILED" || type == "LOAD_CANCELLED" ||
               type == "INVALID_REQUEST") {
      if (payload["requestId"].toInt() == load_request_id_) {
        load_request_id_ = 0;
        SetState(Idle);
      }
      emit Error(tr("The Cast device couldn't play the track (%1)")
                     .arg(payload["reason"].toString(type)));
    }
  }
}

void CastMediaPlayer::ReceiverStatus(const QJsonObject& status) {
  const QJsonObject volume = status["volume"].toObject();
  if (!volume.isEmpty()) {
    const double level = volume["level"].toDouble(volume_);
    const bool muted = volume["muted"].toBool(muted_);
    if (level != volume_ || muted != muted_) {
      volume_ = level;
      muted_ = muted;
      emit VolumeChanged(volume_, muted_);
    }
  }

  // Volume-only updates leave out the applications.
  if (!status.contains("applications")) return;

  for (const QJsonValue& value : status["applications"].toArray()) {
    const QJsonObject app = value.toObject();
    if (app["appId"].toString() != kDefaultMediaReceiverAppId) continue;

    const QString session_id = app["sessionId"].toString();
    if (session_id == session_id_) return;

    session_id_ = session_id;
    transport_id_ = app["transportId"].toString();
    media_session_id_ = 0;
    launching_ = false;
    qLog(Debug) << "Cast media app running, session" << session_id_;
    // Find out whether it's already playing something.
    SendMedia({{"type", "GET_STATUS"}});
    emit Ready();
    return;
  }

  // The app isn't running. While launching, that's the status from before it
  // started.
  if (!session_id_.isEmpty()) {
    qLog(Debug) << "Cast media app stopped";
    AppGone();
  }
}

void CastMediaPlayer::MediaStatus(const QJsonObject& status) {
  const int media_session_id = status["mediaSessionId"].toInt();
  // An older track's status.
  if (media_session_id < media_session_id_) return;
  if (media_session_id == idle_media_session_id_) return;
  media_session_id_ = media_session_id;
  load_request_id_ = 0;

  if (status.contains("currentTime")) {
    position_ms_ = qint64(status["currentTime"].toDouble() * 1000);
    position_updated_.start();
  }
  playback_rate_ = status["playbackRate"].toDouble(1.0);
  const QJsonObject media = status["media"].toObject();
  if (media.contains("duration")) {
    duration_ms_ = qint64(media["duration"].toDouble() * 1000);
  }

  QString player_state = status["playerState"].toString();
  // While loading, devices say IDLE, and put LOADING in the extended status.
  const QString extended_state =
      status["extendedStatus"].toObject()["playerState"].toString();
  if (player_state == "IDLE" && extended_state == "LOADING") {
    player_state = "BUFFERING";
  }

  if (player_state == "PLAYING") {
    SetState(Playing);
  } else if (player_state == "PAUSED") {
    SetState(Paused);
  } else if (player_state == "BUFFERING") {
    SetState(Buffering);
  } else if (player_state == "IDLE" && status.contains("idleReason")) {
    // Without a reason, the track hasn't started yet.
    const QString reason = status["idleReason"].toString();
    idle_media_session_id_ = media_session_id_;
    media_session_id_ = 0;
    SetState(Idle);
    if (reason == "FINISHED") {
      emit TrackEnded();
    } else if (reason == "ERROR") {
      emit Error(tr("The Cast device stopped playing because of an error"));
    }
  }
}

void CastMediaPlayer::SetState(State state) {
  if (state == state_) return;
  state_ = state;
  emit StateChanged(state_);
}

void CastMediaPlayer::AppGone() {
  session_id_.clear();
  transport_id_.clear();
  media_session_id_ = 0;
  load_request_id_ = 0;
  launching_ = false;
  SetState(Idle);
  emit AppStopped();
}
