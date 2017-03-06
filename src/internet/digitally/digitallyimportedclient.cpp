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

#include <qjson/parser.h>

#include <QNetworkReply>
#include <QNetworkRequest>

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
    "http://api.v2.audioaddict.com/v1/%1/mobile/"
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
                                             .toAscii()
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

  QJson::Parser parser;
  QVariantMap data = parser.parse(reply->readAll()).toMap();

  if (!data.contains("subscriptions")) {
    return ret;
  }

  QVariantList subscriptions =
      data.value("subscriptions", QVariantList()).toList();
  if (subscriptions.isEmpty() ||
      subscriptions[0].toMap().value("status").toString() != "active") {
    ret.error_reason_ = tr("You do not have an active subscription");
    return ret;
  }

  if (!data.contains("first_name") || !data.contains("last_name") ||
      !subscriptions[0].toMap().contains("expires_on") ||
      !data.contains("listen_key"))
    return ret;

  ret.success_ = true;
  ret.first_name_ = data["first_name"].toString();
  ret.last_name_ = data["last_name"].toString();
  ret.expires_ = QDateTime::fromString(
      subscriptions[0].toMap()["expires_on"].toString(), Qt::ISODate);
  ret.listen_hash_ = data["listen_key"].toString();
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

  QJson::Parser parser;
  QVariantMap data = parser.parse(reply->readAll()).toMap();

  if (!data.contains("channel_filters")) return ret;

  QVariantList filters = data["channel_filters"].toList();

  for (const QVariant& filter : filters) {
    // Find the filter called "All"
    QVariantMap filter_map = filter.toMap();
    if (filter_map.value("name", QString()).toString() != "All") continue;

    // Add all its stations to the result
    QVariantList channels =
        filter_map.value("channels", QVariantList()).toList();
    for (const QVariant& channel_var : channels) {
      QVariantMap channel_map = channel_var.toMap();

      Channel channel;
      channel.art_url_ = QUrl(channel_map.value("asset_url").toString());
      channel.description_ = channel_map.value("description").toString();
      channel.director_ = channel_map.value("channel_director").toString();
      channel.key_ = channel_map.value("key").toString();
      channel.name_ = channel_map.value("name").toString();
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
