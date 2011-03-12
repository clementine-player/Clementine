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

#include "musicdnsclient.h"
#include "core/network.h"

#include <QCoreApplication>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QtDebug>

const char* MusicDnsClient::kClientId = "c44f70e49000dd7c0d1388bff2bf4152";
const char* MusicDnsClient::kUrl = "http://ofa.musicdns.org/ofa/1/track";

MusicDnsClient::MusicDnsClient(QObject* parent)
  : QObject(parent),
    network_(new NetworkAccessManager(this))
{
}

void MusicDnsClient::Start(int id, const QString& fingerprint, int duration_msec) {
  typedef QPair<QString, QString> Param;

  QList<Param> parameters;
  parameters << Param("alb", "unknown")
             << Param("art", "unknown")
             << Param("brt", "0")
             << Param("cid", kClientId)
             << Param("cvr", QString("%1 %2").arg(QCoreApplication::applicationName(),
                                                  QCoreApplication::applicationVersion()))
             << Param("dur", QString::number(duration_msec))
             << Param("fmt", "unknown")
             << Param("fpt", fingerprint)
             << Param("gnr", "unknown")
             << Param("rmd", "1")
             << Param("tnm", "0")
             << Param("ttl", "unknown")
             << Param("yrr", "0");

  QUrl url(kUrl);
  url.setQueryItems(parameters);
  QNetworkRequest req(url);

  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));
  requests_[reply] = id;
}

void MusicDnsClient::Cancel(int id) {
  requests_.remove(requests_.key(id));
}

void MusicDnsClient::CancelAll() {
  requests_.clear();
}

void MusicDnsClient::RequestFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();
  if (!requests_.contains(reply))
    return;

  int id = requests_.take(reply);

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
    emit Finished(id, QString());
    return;
  }

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == "puid") {
      QString puid = reader.attributes().value("id").toString();
      emit Finished(id, puid);
      return;
    }
  }

  emit Finished(id, QString());
}
