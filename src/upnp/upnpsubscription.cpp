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

#include "upnpsubscription.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QXmlStreamReader>

#include "core/logging.h"
#include "networkremote/streaming/eventcallbacktable.h"
#include "networkremote/streaming/streamitemtable.h"

const int UpnpSubscription::kTimeoutSec = 1800;

namespace {

const int kRequestTimeoutMsec = 5000;
// Renewed this long before it runs out, or a fifth of its time if that's
// shorter.
const int kRenewMarginMsec = 60000;

}  // namespace

UpnpSubscription::UpnpSubscription(
    QNetworkAccessManager* network,
    std::shared_ptr<EventCallbackTable> callbacks, const QUrl& event_url,
    const QHostAddress& renderer, QObject* parent)
    : QObject(parent),
      network_(network),
      callbacks_(callbacks),
      event_url_(event_url),
      renderer_(renderer),
      token_(StreamItemTable::NewToken()),
      renew_timer_(new QTimer(this)) {
  renew_timer_->setSingleShot(true);
  connect(renew_timer_, &QTimer::timeout, this, &UpnpSubscription::Renew);
}

UpnpSubscription::~UpnpSubscription() {
  callbacks_->Remove(token_);
  Unsubscribe();
}

void UpnpSubscription::Start(const QUrl& base) {
  base_ = base;
  callbacks_->Set(token_, renderer_, this);
  Subscribe(false);
}

void UpnpSubscription::Renew() { Subscribe(true); }

void UpnpSubscription::Subscribe(bool renewing) {
  QNetworkRequest request(event_url_);
  request.setTransferTimeout(kRequestTimeoutMsec);
  if (renewing) {
    request.setRawHeader("SID", sid_);
  } else {
    QUrl callback = base_;
    callback.setPath("/e/" + QString::fromLatin1(token_));
    request.setRawHeader(
        "CALLBACK",
        "<" + callback.toString(QUrl::FullyEncoded).toLatin1() + ">");
    request.setRawHeader("NT", "upnp:event");
  }
  request.setRawHeader("TIMEOUT", "Second-" + QByteArray::number(kTimeoutSec));

  QNetworkReply* reply = network_->sendCustomRequest(request, "SUBSCRIBE");
  connect(reply, &QNetworkReply::finished, this, [this, reply, renewing]() {
    reply->deleteLater();
    SubscribeFinished(reply, renewing);
  });
}

void UpnpSubscription::SubscribeFinished(QNetworkReply* reply, bool renewing) {
  const int status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const QByteArray sid = reply->rawHeader("SID");
  if (status != 200 || (!renewing && sid.isEmpty())) {
    const QString reason =
        status ? QString("HTTP %1").arg(status) : reply->errorString();
    qLog(Warning) << "Couldn't" << (renewing ? "renew" : "subscribe to")
                  << event_url_ << ":" << reason;
    sid_.clear();
    // It may have forgotten the subscription, so start again, once.
    if (renewing) {
      Subscribe(false);
    } else {
      emit Failed(reason);
    }
    return;
  }

  if (!sid.isEmpty()) sid_ = sid;
  // "Second-1800", or "infinite".
  const QByteArray timeout = reply->rawHeader("TIMEOUT").toLower();
  bool ok = false;
  qint64 timeout_msec =
      timeout.startsWith("second-") ? timeout.mid(7).toLongLong(&ok) * 1000 : 0;
  if (!ok || timeout_msec <= 0) timeout_msec = kTimeoutSec * 1000ll;
  renew_timer_->start(timeout_msec -
                      qMin<qint64>(kRenewMarginMsec, timeout_msec / 5));

  if (!renewing) {
    qLog(Debug) << "Subscribed to" << event_url_ << "as" << sid_;
    emit Subscribed();
  }
}

void UpnpSubscription::Unsubscribe() {
  renew_timer_->stop();
  if (sid_.isEmpty()) return;
  QNetworkRequest request(event_url_);
  request.setTransferTimeout(kRequestTimeoutMsec);
  request.setRawHeader("SID", sid_);
  sid_.clear();
  // Nothing more to do with the answer.
  QNetworkReply* reply = network_->sendCustomRequest(request, "UNSUBSCRIBE");
  connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
}

void UpnpSubscription::EventReceived(const QByteArray& body) {
  emit Event(ParsePropertySet(body));
}

QMap<QString, QString> UpnpSubscription::ParsePropertySet(
    const QByteArray& body) {
  // <e:propertyset><e:property><Name>value</Name></e:property>...
  QMap<QString, QString> ret;
  QXmlStreamReader reader(body);
  while (reader.readNextStartElement()) {
    if (reader.name() == QLatin1String("propertyset")) continue;
    if (reader.name() != QLatin1String("property")) {
      reader.skipCurrentElement();
      continue;
    }
    while (reader.readNextStartElement()) {
      const QString name = reader.name().toString();
      const QString value = reader.readElementText();
      if (name == "LastChange") {
        const QMap<QString, QString> changed = ParseLastChange(value);
        for (auto it = changed.begin(); it != changed.end(); ++it) {
          ret[it.key()] = it.value();
        }
      } else {
        ret[name] = value;
      }
    }
  }
  return ret;
}

QMap<QString, QString> UpnpSubscription::ParseLastChange(const QString& xml) {
  // <Event><InstanceID val="0"><TransportState val="PLAYING"/>...
  QMap<QString, QString> ret;
  QXmlStreamReader reader(xml);
  while (reader.readNextStartElement()) {
    if (reader.name() == QLatin1String("Event")) continue;
    if (reader.name() != QLatin1String("InstanceID") ||
        reader.attributes().value("val") != QLatin1String("0")) {
      reader.skipCurrentElement();
      continue;
    }
    while (reader.readNextStartElement()) {
      const QXmlStreamAttributes attributes = reader.attributes();
      // Vendors' own, such as Sonos's r:NextTrackURI, keep their prefix.
      const QString name = reader.qualifiedName().toString();
      const bool other_channel =
          attributes.hasAttribute("channel") &&
          attributes.value("channel") != QLatin1String("Master");
      if (!other_channel) ret[name] = attributes.value("val").toString();
      reader.skipCurrentElement();
    }
  }
  return ret;
}
