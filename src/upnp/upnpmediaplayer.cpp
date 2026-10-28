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

#include "upnpmediaplayer.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QTimer>
#include <QXmlStreamWriter>

#include "core/logging.h"

const int UpnpMediaPlayer::kPollIntervalMsec = 1000;
const int UpnpMediaPlayer::kEndMarginMsec = 3000;

namespace {

const char kDidlNamespace[] = "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/";
const char kDcNamespace[] = "http://purl.org/dc/elements/1.1/";
const char kUpnpNamespace[] = "urn:schemas-upnp-org:metadata-1-0/upnp/";

// A renderer that's stopped, without having played, for this many polls after
// being told to play couldn't play the track.
const int kStoppedPollsToFail = 2;

}  // namespace

UpnpMediaPlayer::UpnpMediaPlayer(QObject* parent)
    : QObject(parent),
      network_(new QNetworkAccessManager(this)),
      poll_timer_(new QTimer(this)),
      in_flight_(false),
      generation_(0),
      loaded_(false),
      started_(false),
      play_sent_(false),
      want_play_(false),
      stopped_polls_(0),
      state_(Idle),
      position_ms_(0),
      last_playing_position_ms_(0),
      pending_seek_ms_(-1),
      duration_ms_(0),
      next_duration_ms_(0),
      next_unsupported_(false) {
  poll_timer_->setInterval(kPollIntervalMsec);
  connect(poll_timer_, &QTimer::timeout, this, &UpnpMediaPlayer::Poll);
}

void UpnpMediaPlayer::SetDevice(const UpnpDevice& device) { device_ = device; }

void UpnpMediaPlayer::Load(const UpnpMediaInfo& info, qint64 start_ms,
                           bool autoplay) {
  ++generation_;
  queue_.clear();
  loaded_ = true;
  uri_ = info.url.toString(QUrl::FullyEncoded);
  started_ = false;
  play_sent_ = false;
  want_play_ = autoplay;
  stopped_polls_ = 0;
  position_ms_ = start_ms;
  position_updated_.start();
  last_playing_position_ms_ = start_ms;
  pending_seek_ms_ = -1;
  duration_ms_ = info.duration_ms;
  // A new URI replaces anything queued after the old one.
  next_uri_.clear();
  SetState(autoplay ? Buffering : Idle);

  Enqueue(SetUri, device_.av_transport, "SetAVTransportURI",
          {{"CurrentURI", uri_}, {"CurrentURIMetaData", Didl(info)}});
  if (start_ms > 0) {
    Enqueue(LoadSeek, device_.av_transport, "Seek",
            {{"Unit", "REL_TIME"}, {"Target", FormatTime(start_ms)}});
  }
  if (autoplay)
    Enqueue(LoadPlay, device_.av_transport, "Play", {{"Speed", "1"}});
  poll_timer_->start();
}

void UpnpMediaPlayer::Play() {
  if (!loaded_) return;
  want_play_ = true;
  // Until it's started, failing to play is failing to play the track.
  Enqueue(started_ ? Command : LoadPlay, device_.av_transport, "Play",
          {{"Speed", "1"}});
}

void UpnpMediaPlayer::Pause() {
  if (!loaded_) return;
  want_play_ = false;
  Enqueue(Command, device_.av_transport, "Pause");
}

void UpnpMediaPlayer::Stop() {
  if (!loaded_) return;
  Unload();
  Enqueue(Command, device_.av_transport, "Stop");
}

void UpnpMediaPlayer::Seek(qint64 position_ms) {
  if (!loaded_) return;
  if (pending_seek_ms_ >= 0) {
    // This would fail too.
    pending_seek_ms_ = position_ms;
  } else {
    Enqueue(Command, device_.av_transport, "Seek",
            {{"Unit", "REL_TIME"}, {"Target", FormatTime(position_ms)}});
  }
  position_ms_ = position_ms;
  position_updated_.start();
  last_playing_position_ms_ = position_ms;
}

void UpnpMediaPlayer::SetNext(const UpnpMediaInfo& info) {
  if (!loaded_ || next_unsupported_) return;
  next_uri_ = info.url.toString(QUrl::FullyEncoded);
  next_duration_ms_ = info.duration_ms;
  Enqueue(SetNextUri, device_.av_transport, "SetNextAVTransportURI",
          {{"NextURI", next_uri_}, {"NextURIMetaData", Didl(info)}});
}

void UpnpMediaPlayer::SetVolume(int volume) {
  const QString value = QString::number(qBound(0, volume, 100));
  // Only the latest matters.
  for (Request& request : queue_) {
    if (request.kind == Volume) {
      request.action.args.last().second = value;
      return;
    }
  }
  if (device_.group_rendering_control.is_valid()) {
    Enqueue(Volume, device_.group_rendering_control, "SetGroupVolume",
            {{"DesiredVolume", value}});
  } else if (device_.rendering_control.is_valid()) {
    Enqueue(Volume, device_.rendering_control, "SetVolume",
            {{"Channel", "Master"}, {"DesiredVolume", value}});
  }
}

qint64 UpnpMediaPlayer::position_ms() const {
  if (state_ != Playing) return position_ms_;
  return position_ms_ + position_updated_.elapsed();
}

void UpnpMediaPlayer::Unload() {
  ++generation_;
  // Only what's already been sent is still answered.
  queue_.clear();
  loaded_ = false;
  uri_.clear();
  started_ = false;
  play_sent_ = false;
  want_play_ = false;
  pending_seek_ms_ = -1;
  next_uri_.clear();
  poll_timer_->stop();
  SetState(Idle);
}

void UpnpMediaPlayer::Enqueue(Kind kind, const UpnpService& service,
                              const QString& name, UpnpSoap::Args args) {
  Request request;
  request.kind = kind;
  request.generation = generation_;
  request.action.control_url = service.control_url;
  request.action.service_type = service.type;
  request.action.name = name;
  // Every AVTransport and RenderingControl action has one.
  args.prepend({"InstanceID", "0"});
  request.action.args = args;
  queue_ << request;
  SendNext();
}

bool UpnpMediaPlayer::Queued(Kind kind) const {
  if (in_flight_ && current_.kind == kind) return true;
  for (const Request& request : queue_) {
    if (request.kind == kind) return true;
  }
  return false;
}

void UpnpMediaPlayer::SendNext() {
  if (in_flight_ || queue_.isEmpty()) return;
  current_ = queue_.takeFirst();
  in_flight_ = true;
  Send(current_.action);
}

void UpnpMediaPlayer::Send(const UpnpSoap::Action& action) {
  QNetworkReply* reply = UpnpSoap::Send(network_, action);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();
    Finished(UpnpSoap::ParseReply(reply));
  });
}

void UpnpMediaPlayer::Finished(const UpnpSoap::Response& response) {
  in_flight_ = false;
  const Request request = current_;
  const bool current = loaded_ && request.generation == generation_;

  if (!response.ok) {
    qLog(Warning) << "UPnP" << request.action.name
                  << "failed:" << response.error;
  }

  if (!response.ok && response.http_status == 0) {
    // Nothing more will get through.
    if (current) {
      Unload();
      emit Lost(response.error);
    }
    SendNext();
    return;
  }

  switch (request.kind) {
    case SetUri:
    case LoadPlay:
      if (!current) break;
      if (!response.ok) {
        Unload();
        emit Error(tr("The renderer couldn't play it: %1").arg(response.error));
      } else if (request.kind == LoadPlay) {
        play_sent_ = true;
      }
      break;
    case LoadSeek:
      if (current && !response.ok) {
        // Not a reason to give up on it: try again once it's started.
        pending_seek_ms_ = ParseTime(request.action.args.last().second);
      }
      break;
    case SetNextUri:
      if (current && !response.ok) {
        // The next track is loaded once this one ends instead.
        next_uri_.clear();
        // Invalid Action, and Optional Action Not Implemented.
        if (response.error_code == 401 || response.error_code == 602) {
          next_unsupported_ = true;
        }
      }
      break;
    case Command:
    case Volume:
      // Logged above; the next poll says what the renderer's doing.
      break;
    case PollPosition:
      if (current && response.ok) PositionInfo(response.values);
      break;
    case PollTransport:
      if (current && response.ok) TransportInfo(response.values);
      break;
  }
  SendNext();
}

void UpnpMediaPlayer::Poll() {
  if (!loaded_) return;
  // Position first, so that a stop is judged by the latest position.
  if (!Queued(PollPosition)) {
    Enqueue(PollPosition, device_.av_transport, "GetPositionInfo");
  }
  if (!Queued(PollTransport)) {
    Enqueue(PollTransport, device_.av_transport, "GetTransportInfo");
  }
}

void UpnpMediaPlayer::PositionInfo(const QMap<QString, QString>& values) {
  const QString track_uri = values.value("TrackURI");
  if (started_ && !next_uri_.isEmpty() && !track_uri.isEmpty() &&
      QUrl(track_uri) == QUrl(next_uri_)) {
    uri_ = next_uri_;
    next_uri_.clear();
    duration_ms_ = next_duration_ms_;
    position_ms_ = 0;
    position_updated_.start();
    last_playing_position_ms_ = 0;
    pending_seek_ms_ = -1;
    stopped_polls_ = 0;
    emit NextStarted();
  }
  if (started_ && !track_uri.isEmpty() && QUrl(track_uri) != QUrl(uri_)) {
    qLog(Info) << "The renderer is playing" << track_uri << "instead of"
               << uri_;
    Unload();
    emit Lost(tr("Something else is playing on it now"));
    return;
  }

  const qint64 position = ParseTime(values.value("RelTime"));
  // Still at the start, until it can seek to where it should be.
  if (position >= 0 && pending_seek_ms_ < 0) {
    position_ms_ = position;
    position_updated_.start();
  }
  const qint64 duration = ParseTime(values.value("TrackDuration"));
  if (duration > 0) duration_ms_ = duration;
}

void UpnpMediaPlayer::TransportInfo(const QMap<QString, QString>& values) {
  const QString status = values.value("CurrentTransportStatus");
  if (!status.isEmpty() && status != "OK") {
    qLog(Warning) << "UPnP transport status" << status;
    if (!started_) {
      Unload();
      emit Error(tr("The renderer couldn't play it: %1").arg(status));
      return;
    }
  }

  const QString state = values.value("CurrentTransportState");
  if (state == "PLAYING") {
    started_ = true;
    stopped_polls_ = 0;
    last_playing_position_ms_ = position_ms_;
    SetState(Playing);
  } else if (state == "PAUSED_PLAYBACK") {
    started_ = true;
    SetState(Paused);
  } else if (state == "TRANSITIONING") {
    SetState(Buffering);
  } else if (state == "STOPPED" || state == "NO_MEDIA_PRESENT") {
    // Loaded paused, or paused before it started.
    if (!want_play_) return;

    if (started_) {
      const bool ended = duration_ms_ <= 0 || last_playing_position_ms_ >=
                                                  duration_ms_ - kEndMarginMsec;
      Unload();
      if (ended) {
        emit TrackEnded();
      } else {
        emit Lost(tr("It was stopped from somewhere else"));
      }
    } else if (play_sent_ && ++stopped_polls_ >= kStoppedPollsToFail) {
      Unload();
      emit Error(tr("The renderer couldn't play it"));
    }
  }

  if (started_ && pending_seek_ms_ >= 0) {
    const qint64 position_ms = pending_seek_ms_;
    pending_seek_ms_ = -1;
    Seek(position_ms);
  }
}

void UpnpMediaPlayer::SetState(State state) {
  if (state == state_) return;
  state_ = state;
  emit StateChanged(state_);
}

QString UpnpMediaPlayer::Didl(const UpnpMediaInfo& info) {
  // Without any codecs= parameter.
  const QString mime_type = info.content_type.section(';', 0, 0).trimmed();

  QString ret;
  QXmlStreamWriter writer(&ret);
  writer.writeDefaultNamespace(kDidlNamespace);
  writer.writeNamespace(kDcNamespace, "dc");
  writer.writeNamespace(kUpnpNamespace, "upnp");
  writer.writeStartElement(kDidlNamespace, "DIDL-Lite");
  writer.writeStartElement(kDidlNamespace, "item");
  writer.writeAttribute("id", "-1");
  writer.writeAttribute("parentID", "-1");
  writer.writeAttribute("restricted", "true");

  writer.writeTextElement(kDcNamespace, "title", info.title);
  if (!info.artist.isEmpty()) {
    writer.writeTextElement(kDcNamespace, "creator", info.artist);
    writer.writeTextElement(kUpnpNamespace, "artist", info.artist);
  }
  if (!info.album.isEmpty()) {
    writer.writeTextElement(kUpnpNamespace, "album", info.album);
  }
  if (!info.image_url.isEmpty()) {
    writer.writeTextElement(kUpnpNamespace, "albumArtURI",
                            info.image_url.toString(QUrl::FullyEncoded));
  }
  writer.writeTextElement(kUpnpNamespace, "class",
                          "object.item.audioItem.musicTrack");

  writer.writeStartElement(kDidlNamespace, "res");
  writer.writeAttribute("protocolInfo",
                        QString("http-get:*:%1:*").arg(mime_type));
  if (info.duration_ms > 0) {
    writer.writeAttribute("duration", FormatTime(info.duration_ms));
  }
  writer.writeCharacters(info.url.toString(QUrl::FullyEncoded));
  writer.writeEndElement();

  writer.writeEndElement();  // item
  writer.writeEndElement();  // DIDL-Lite
  return ret;
}

QString UpnpMediaPlayer::FormatTime(qint64 ms) {
  const qint64 seconds = qMax(Q_INT64_C(0), ms) / 1000;
  return QString("%1:%2:%3")
      .arg(seconds / 3600)
      .arg((seconds / 60) % 60, 2, 10, QChar('0'))
      .arg(seconds % 60, 2, 10, QChar('0'));
}

qint64 UpnpMediaPlayer::ParseTime(const QString& time) {
  // "H+:MM:SS", maybe with a fraction of a second.
  const QStringList parts = time.split(':');
  if (parts.size() != 3) return -1;
  bool ok_h, ok_m, ok_s;
  const qint64 hours = parts[0].toLongLong(&ok_h);
  const qint64 minutes = parts[1].toLongLong(&ok_m);
  const double seconds = parts[2].toDouble(&ok_s);
  if (!ok_h || !ok_m || !ok_s || hours < 0 || minutes < 0 || seconds < 0) {
    return -1;
  }
  return (hours * 3600 + minutes * 60) * 1000 + qint64(seconds * 1000);
}
