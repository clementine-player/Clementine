/* This file is part of Clementine.
   Copyright 2011-2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "digitallyimportedclient.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "core/logging.h"
#include "core/network.h"

// The API used here is undocumented - it was reverse engineered by watching
// calls made by the sky.fm (now RadioTunes) android app:
// https://market.android.com/details?id=com.audioaddict.sky

const char* DigitallyImportedClient::kApiUsername = "ephemeron";
const char* DigitallyImportedClient::kApiPassword = "dayeiph0ne@pp";

const char* DigitallyImportedClient::kAuthUrl =
    "http://api.audioaddict.com/v1/%1/members/authenticate";

const char* DigitallyImportedClient::kChannelListUrl =
    "http://api.audioaddict.com/v1/%1/mobile/"
    "batch_update?asset_group_key=mobile_icons&stream_set_key=";

DigitallyImportedClient::DigitallyImportedClient(const QString& service_name,
                                                 QObject* parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)),
      service_name_(service_name) {}

void DigitallyImportedClient::SetAuthorisationHeader(
    QNetworkRequest* req) const {
  req->setRawHeader("Authorization", "Basic " +
                                         QString("%1:%2")
                                             .arg(kApiUsername, kApiPassword)
                                             .toLatin1()
                                             .toBase64());
}

QNetworkReply* DigitallyImportedClient::Auth(const QString& username,
                                             const QString& password) {
  QNetworkRequest req(QUrl(QString(kAuthUrl).arg(service_name_)));
  SetAuthorisationHeader(&req);

  QByteArray postdata = "username=" + QUrl::toPercentEncoding(username) +
                        "&password=" + QUrl::toPercentEncoding(password);

  return network_->post(req, postdata);
}

DigitallyImportedClient::AuthReply DigitallyImportedClient::ParseAuthReply(
    QNetworkReply* reply) const {
  AuthReply ret;
  ret.success_ = false;
  ret.error_reason_ = tr("Unknown error");

  const int http_status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (http_status == 403) {
    ret.error_reason_ = reply->readAll();
    return ret;
  } else if (http_status != 200) {
    return ret;
  }

  QJsonObject json_root_object = QJsonDocument::fromJson(reply->readAll()).object();

  if (json_root_object["subscriptions"].isUndefined()) {
    return ret;
  }

  QJsonArray json_subscriptions = json_root_object["subscriptions"].toArray();
  if (json_subscriptions.isEmpty() ||
      json_subscriptions[0].toObject()["status"].toString() != "active") {
    ret.error_reason_ = tr("You do not have an active subscription");
    return ret;
  }

  if (json_root_object["first_name"].isUndefined() || json_root_object["last_name"].isUndefined() ||
      json_subscriptions[0].toObject()["expires_on"].isUndefined() ||
      json_root_object["listen_key"].isUndefined())
    return ret;

  ret.success_ = true;
  ret.first_name_ = json_root_object["first_name"].toString();
  ret.last_name_ = json_root_object["last_name"].toString();
  ret.expires_ = QDateTime::fromString(
      json_subscriptions[0].toObject()["expires_on"].toString(), Qt::ISODate);
  ret.listen_hash_ = json_root_object["listen_key"].toString();
  return ret;
}

QNetworkReply* DigitallyImportedClient::GetChannelList() {
  // QNetworkRequest req(QUrl(QString(kChannelListUrl)));
  QNetworkRequest req(QUrl(QString(kChannelListUrl).arg(service_name_)));
  SetAuthorisationHeader(&req);

  return network_->get(req);
}

DigitallyImportedClient::ChannelList DigitallyImportedClient::ParseChannelList(
    QNetworkReply* reply) const {
  ChannelList ret;

  QJsonObject json_root_object = QJsonDocument::fromJson(reply->readAll()).object();

  if (json_root_object["channel_filters"].isUndefined()) return ret;

  QJsonArray json_filters = json_root_object["channel_filters"].toArray();

  for (const QJsonValue & filter: json_filters) {
    // Find the filter called "All"
    QJsonObject json_filter = filter.toObject();
    if (json_filter["name"].toString() != "All") continue;

    // Add all its stations to the result
    QJsonArray json_channels = json_filter["channels"].toArray();
    for (const QJsonValue& channel_var : json_channels) {
      QJsonObject json_channel = channel_var.toObject();

      Channel channel;
      channel.art_url_ = QUrl(json_channel["asset_url"].toString());
      channel.description_ = json_channel["description"].toString();
      channel.director_ = json_channel["channel_director"].toString();
      channel.key_ = json_channel["key"].toString();
      channel.name_ = json_channel["name"].toString();
      ret << channel;
    }

    break;
  }

  return ret;
}

QDataStream& operator<<(QDataStream& out,
                        const DigitallyImportedClient::Channel& channel) {
  out << channel.art_url_ << channel.director_ << channel.description_
      << channel.name_ << channel.key_;
  return out;
}

QDataStream& operator>>(QDataStream& in,
                        DigitallyImportedClient::Channel& channel) {
  in >> channel.art_url_ >> channel.director_ >> channel.description_ >>
      channel.name_ >> channel.key_;
  return in;
}
