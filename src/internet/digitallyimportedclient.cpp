/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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
#include "core/network.h"

#include <qjson/parser.h>

#include <QNetworkReply>
#include <QNetworkRequest>

// The API used here is undocumented - it was reverse engineered by watching
// calls made by the sky.fm android app:
// https://market.android.com/details?id=com.audioaddict.sky

const char* DigitallyImportedClient::kApiUsername = "ephemeron";
const char* DigitallyImportedClient::kApiPassword = "dayeiph0ne@pp";

const char* DigitallyImportedClient::kAuthUrl =
    "http://api.audioaddict.com/%1/premium/auth";


DigitallyImportedClient::DigitallyImportedClient(const QString& service_name, QObject* parent)
  : QObject(parent),
    network_(new NetworkAccessManager(this)),
    service_name_(service_name)
{
}

QNetworkReply* DigitallyImportedClient::Auth(const QString& username,
                                             const QString& password) {
  QNetworkRequest req(QUrl(QString(kAuthUrl).arg(service_name_)));
  req.setRawHeader("Authorization",
                   "Basic " + QString("%1:%2").arg(kApiUsername, kApiPassword)
                      .toAscii().toBase64());

  QByteArray postdata = "username=" + QUrl::toPercentEncoding(username) +
                       "&password=" + QUrl::toPercentEncoding(password);

  return network_->post(req, postdata);
}

DigitallyImportedClient::AuthReply
DigitallyImportedClient::ParseAuthReply(QNetworkReply* reply) const {
  AuthReply ret;
  ret.success_ = false;
  ret.error_reason_ = tr("Unknown error");

  QJson::Parser parser;
  QVariantMap data = parser.parse(reply).toMap();

  if (data.value("status", QString()).toString() != "OK" ||
      !data.contains("user")) {
    ret.error_reason_ = data.value("reason", ret.error_reason_).toString();
    return ret;
  }

  QVariantMap user = data["user"].toMap();
  if (!user.contains("first_name") ||
      !user.contains("last_name") ||
      !user.contains("expires") ||
      !user.contains("listen_hash"))
    return ret;

  ret.success_ = true;
  ret.first_name_ = user["first_name"].toString();
  ret.last_name_ = user["last_name"].toString();
  ret.expires_ = QDateTime::fromString(user["expires"].toString(), Qt::ISODate);
  ret.listen_hash_ = user["listen_hash"].toString();
  return ret;
}
