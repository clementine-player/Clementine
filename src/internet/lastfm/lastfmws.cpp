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

#include "lastfmws.h"

#include <QCryptographicHash>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QUrlQuery>

namespace LastFmWs {

QString api_key;
QString shared_secret;
QString username;
QString session_key;

namespace {
const char* kEndpoint = "https://ws.audioscrobbler.com/2.0/";
QNetworkAccessManager* g_network = nullptr;

QUrlQuery ToQuery(const QMap<QString, QString>& params) {
  QUrlQuery query;
  for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
    query.addQueryItem(it.key(), it.value());
  }
  return query;
}
}  // namespace

void SetNetworkAccessManager(QNetworkAccessManager* network) {
  delete g_network;
  g_network = network;
}

QByteArray Sign(const QMap<QString, QString>& params) {
  // QMap already iterates in ascending key order, which is exactly the
  // ordering the signing spec requires - no separate sort needed.
  QString to_sign;
  for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
    to_sign += it.key();
    to_sign += it.value();
  }
  to_sign += shared_secret;
  return QCryptographicHash::hash(to_sign.toUtf8(), QCryptographicHash::Md5)
      .toHex();
}

QNetworkReply* Get(QMap<QString, QString> params) {
  params["api_key"] = api_key;

  QUrl url(kEndpoint);
  url.setQuery(ToQuery(params));
  return g_network->get(QNetworkRequest(url));
}

QNetworkReply* Post(QMap<QString, QString> params) {
  params["api_key"] = api_key;
  if (!session_key.isEmpty()) {
    params["sk"] = session_key;
  }
  params["api_sig"] = Sign(params);

  QNetworkRequest request((QUrl(kEndpoint)));
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  return g_network->post(
      request, ToQuery(params).toString(QUrl::FullyEncoded).toUtf8());
}

}  // namespace LastFmWs
