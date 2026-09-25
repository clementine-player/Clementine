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

#include "plexnotificationlistener.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>

#include "core/logging.h"
#include "internet/plex/plexservice.h"

namespace {

// The server pings every 10 seconds, so this much silence means the stream is
// dead even if the socket hasn't noticed.
const int kWatchdogMsec = 30000;
const int kInitialBackoffMsec = 5000;
const int kMaxBackoffMsec = 5 * 60 * 1000;

// Plex metadata types for music.
const int kTypeArtist = 8;
const int kTypeAlbum = 9;
const int kTypeTrack = 10;

// TimelineEntry states.
const int kStateDone = 5;
const int kStateDeleted = 9;

// A notification field may hold one object or an array of them.
QJsonArray AsArray(const QJsonValue& value) {
  if (value.isArray()) return value.toArray();
  if (value.isObject()) return QJsonArray{value};
  return QJsonArray();
}

}  // namespace

PlexNotificationListener::PlexNotificationListener(PlexService* service,
                                                   QObject* parent)
    : QObject(parent),
      service_(service),
      reply_(nullptr),
      watchdog_(new QTimer(this)),
      reconnect_timer_(new QTimer(this)),
      backoff_msec_(kInitialBackoffMsec),
      running_(false) {
  watchdog_->setSingleShot(true);
  watchdog_->setInterval(kWatchdogMsec);
  connect(watchdog_, SIGNAL(timeout()), SLOT(StreamEnded()));

  reconnect_timer_->setSingleShot(true);
  connect(reconnect_timer_, SIGNAL(timeout()), SLOT(Reconnect()));
}

PlexNotificationListener::~PlexNotificationListener() { Stop(); }

void PlexNotificationListener::Start() {
  running_ = true;
  backoff_msec_ = kInitialBackoffMsec;
  Reconnect();
}

void PlexNotificationListener::Stop() {
  running_ = false;
  watchdog_->stop();
  reconnect_timer_->stop();
  if (reply_) {
    QNetworkReply* reply = reply_;
    reply_ = nullptr;
    reply->disconnect(this);
    reply->abort();
    reply->deleteLater();
  }
}

void PlexNotificationListener::Reconnect() {
  if (!running_) return;
  if (reply_) {
    reply_->disconnect(this);
    reply_->abort();
    reply_->deleteLater();
  }

  parser_ = PlexEventStreamParser();

  QUrlQuery query;
  query.addQueryItem("filters", "activity,timeline");
  reply_ = service_->Get("/:/eventsource/notifications", query);
  connect(reply_, SIGNAL(readyRead()), SLOT(ReadyRead()));
  connect(reply_, SIGNAL(finished()), SLOT(StreamEnded()));
  watchdog_->start();
}

void PlexNotificationListener::ReadyRead() {
  watchdog_->start();
  backoff_msec_ = kInitialBackoffMsec;

  for (const PlexServerEvent& event : parser_.Feed(reply_->readAll())) {
    QStringList item_ids;
    if (IsLibraryChange(event, &item_ids)) {
      qLog(Debug) << "Plex library change:" << event.event << item_ids
                  << event.data.left(300);
      emit LibraryChanged(item_ids);
    }
  }
}

void PlexNotificationListener::StreamEnded() {
  if (!reply_) return;
  qLog(Debug) << "Plex event stream ended:" << reply_->errorString();
  reply_->disconnect(this);
  reply_->abort();
  reply_->deleteLater();
  reply_ = nullptr;
  watchdog_->stop();
  ScheduleReconnect();
}

void PlexNotificationListener::ScheduleReconnect() {
  if (!running_) return;
  reconnect_timer_->start(backoff_msec_);
  backoff_msec_ = qMin(backoff_msec_ * 2, kMaxBackoffMsec);
}

bool PlexNotificationListener::IsLibraryChange(const PlexServerEvent& event,
                                               QStringList* item_ids) {
  if (event.data.isEmpty()) return false;

  QJsonObject obj = QJsonDocument::fromJson(event.data).object();
  // The websocket feed wraps notifications in a container; accept both forms.
  if (obj.contains("NotificationContainer")) {
    obj = obj["NotificationContainer"].toObject();
  }

  bool changed = false;

  for (const QJsonValue& value : AsArray(obj["ActivityNotification"])) {
    const QJsonObject notification = value.toObject();
    const QString type = notification["Activity"].toObject()["type"].toString();
    if (notification["event"].toString() == "ended" &&
        type.startsWith("library.")) {
      changed = true;
    }
  }

  for (const QJsonValue& value : AsArray(obj["TimelineEntry"])) {
    const QJsonObject entry = value.toObject();
    const int type = entry["type"].toInt();
    const int state = entry["state"].toInt();
    if (type != kTypeArtist && type != kTypeAlbum && type != kTypeTrack) {
      continue;
    }
    if (state != kStateDone && state != kStateDeleted) continue;

    changed = true;
    const QJsonValue id = entry["itemID"];
    item_ids->append(id.isString() ? id.toString()
                                   : QString::number(id.toInteger()));
  }

  return changed;
}
