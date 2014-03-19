/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "acoustidclient.h"

#include <QCoreApplication>
#include <QNetworkReply>

#include <qjson/parser.h>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/timeconstants.h"

const char* AcoustidClient::kClientId = "qsZGpeLx";
const char* AcoustidClient::kUrl = "http://api.acoustid.org/v2/lookup";
const int AcoustidClient::kDefaultTimeout = 5000;  // msec

AcoustidClient::AcoustidClient(QObject* parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)),
      timeouts_(new NetworkTimeouts(kDefaultTimeout, this)) {}

void AcoustidClient::SetTimeout(int msec) { timeouts_->SetTimeout(msec); }

void AcoustidClient::Start(int id, const QString& fingerprint,
                           int duration_msec) {
  typedef QPair<QString, QString> Param;

  QList<Param> parameters;
  parameters << Param("format", "json") << Param("client", kClientId)
             << Param("duration", QString::number(duration_msec / kMsecPerSec))
             << Param("meta", "recordingids")
             << Param("fingerprint", fingerprint);

  QUrl url(kUrl);
  url.setQueryItems(parameters);
  QNetworkRequest req(url);

  QNetworkReply* reply = network_->get(req);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestFinished(QNetworkReply*, int)), reply, id);
  requests_[id] = reply;

  timeouts_->AddReply(reply);
}

void AcoustidClient::Cancel(int id) { delete requests_.take(id); }

void AcoustidClient::CancelAll() {
  qDeleteAll(requests_.values());
  requests_.clear();
}

void AcoustidClient::RequestFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();
  requests_.remove(id);

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() !=
      200) {
    emit Finished(id, QString());
    return;
  }

  QJson::Parser parser;
  bool ok = false;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    emit Finished(id, QString());
    return;
  }

  QString status = result["status"].toString();
  if (status != "ok") {
    emit Finished(id, QString());
    return;
  }
  QVariantList results = result["results"].toList();
  for (const QVariant& v : results) {
    QVariantMap r = v.toMap();
    if (r.contains("recordings")) {
      QVariantList recordings = r["recordings"].toList();
      for (const QVariant& recording : recordings) {
        QVariantMap o = recording.toMap();
        if (o.contains("id")) {
          emit Finished(id, o["id"].toString());
          return;
        }
      }
    }
  }

  emit Finished(id, QString());
}
