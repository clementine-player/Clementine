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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QStringList>
#include <QUrlQuery>
#include <algorithm>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/timeconstants.h"

const char* AcoustidClient::kClientId = "qsZGpeLx";
const char* AcoustidClient::kUrl = "https://api.acoustid.org/v2/lookup";
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
             << Param("meta", "recordingids+sources")
             << Param("fingerprint", fingerprint);

  QUrl url(kUrl);
  QUrlQuery url_query;
  url_query.setQueryItems(parameters);
  url.setQuery(url_query);
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

namespace {
// Struct used when extracting results in RequestFinished
struct IdSource {
  IdSource(const QString& id, int source) : id_(id), nb_sources_(source) {}

  bool operator<(const IdSource& other) const {
    // We want the items with more sources to be at the beginning of the list
    return nb_sources_ > other.nb_sources_;
  }

  QString id_;
  int nb_sources_;
};
}  // namespace

void AcoustidClient::RequestFinished(QNetworkReply* reply, int request_id) {
  reply->deleteLater();
  requests_.remove(request_id);

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() !=
      200) {
    emit Finished(request_id, QStringList());
    return;
  }

  QJsonParseError error;
  QJsonDocument json_document =
      QJsonDocument::fromJson(reply->readAll(), &error);

  if (error.error != QJsonParseError::NoError) {
    emit Finished(request_id, QStringList());
    return;
  }

  QJsonObject json_object = json_document.object();

  QString status = json_object["status"].toString();
  if (status != "ok") {
    emit Finished(request_id, QStringList());
    return;
  }

  // Get the results:
  // -in a first step, gather ids and their corresponding number of sources
  // -then sort results by number of sources (the results are originally
  //  unsorted but results with more sources are likely to be more accurate)
  // -keep only the ids, as sources where useful only to sort the results
  QJsonArray json_results = json_object["results"].toArray();

  // List of <id, nb of sources> pairs
  QList<IdSource> id_source_list;

  for (const QJsonValue& v : json_results) {
    QJsonObject r = v.toObject();
    if (r.contains("recordings")) {
      QJsonArray json_recordings = r["recordings"].toArray();
      for (const QJsonValue& recording : json_recordings) {
        QJsonObject o = recording.toObject();
        if (o.contains("id")) {
          id_source_list << IdSource(o["id"].toString(), o["sources"].toInt());
        }
      }
    }
  }

  std::stable_sort(id_source_list.begin(), id_source_list.end());

  QList<QString> id_list;
  for (const IdSource& is : id_source_list) {
    id_list << is.id_;
  }

  emit Finished(request_id, id_list);
}
