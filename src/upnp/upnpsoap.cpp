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

#include "upnpsoap.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace {

// Devices answer within a few hundred milliseconds when they're there.
const int kTimeoutMsec = 5000;

// Reads the children of the current element into |values|.
void ReadValues(QXmlStreamReader* reader, QMap<QString, QString>* values) {
  while (reader->readNextStartElement()) {
    const QString name = reader->name().toString();
    (*values)[name] =
        reader->readElementText(QXmlStreamReader::IncludeChildElements);
  }
}

}  // namespace

namespace UpnpSoap {

QByteArray Envelope(const Action& action) {
  QByteArray ret;
  QXmlStreamWriter writer(&ret);
  writer.writeStartDocument();
  writer.writeNamespace("http://schemas.xmlsoap.org/soap/envelope/", "s");
  writer.writeStartElement("http://schemas.xmlsoap.org/soap/envelope/",
                           "Envelope");
  writer.writeAttribute("http://schemas.xmlsoap.org/soap/envelope/",
                        "encodingStyle",
                        "http://schemas.xmlsoap.org/soap/encoding/");
  writer.writeStartElement("http://schemas.xmlsoap.org/soap/envelope/", "Body");
  // Declared on the action itself, where every control point puts it, rather
  // than wherever QXmlStreamWriter would.
  writer.writeStartElement("u:" + action.name);
  writer.writeAttribute("xmlns:u", action.service_type);
  for (const auto& arg : action.args) {
    // Unqualified, as the UPnP spec has them.
    writer.writeTextElement(arg.first, arg.second);
  }
  writer.writeEndDocument();
  return ret;
}

Response Parse(int http_status, const QByteArray& body,
               const QString& network_error) {
  Response ret;
  ret.http_status = http_status;

  QXmlStreamReader reader(body);
  bool in_body = false;
  while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::Invalid) {
    if (!reader.isStartElement()) continue;
    if (!in_body) {
      in_body = reader.name() == QLatin1String("Body");
      continue;
    }

    if (reader.name() == QLatin1String("Fault")) {
      // <detail><UPnPError><errorCode>701</errorCode>...
      QMap<QString, QString> fault;
      while (!reader.atEnd() &&
             reader.readNext() != QXmlStreamReader::Invalid) {
        if (reader.isStartElement() &&
            reader.name() == QLatin1String("UPnPError")) {
          ReadValues(&reader, &fault);
          break;
        }
      }
      ret.error_code = fault.value("errorCode").toInt();
      ret.error = fault.value("errorDescription");
      if (ret.error.isEmpty()) {
        ret.error = QString("UPnP error %1").arg(ret.error_code);
      }
      return ret;
    }

    // <u:PlayResponse>, with the out arguments as children.
    ReadValues(&reader, &ret.values);
    ret.ok = http_status == 200;
    break;
  }

  if (!ret.ok) {
    if (!network_error.isEmpty()) {
      ret.error = network_error;
    } else if (http_status != 200) {
      ret.error = QString("HTTP status %1").arg(http_status);
    } else {
      ret.error = "Unreadable response";
    }
  }
  return ret;
}

QNetworkReply* Send(QNetworkAccessManager* network, const Action& action) {
  QNetworkRequest request(action.control_url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "text/xml; charset=\"utf-8\"");
  request.setRawHeader(
      "SOAPACTION",
      QString("\"%1#%2\"").arg(action.service_type, action.name).toUtf8());
  request.setTransferTimeout(kTimeoutMsec);
  return network->post(request, Envelope(action));
}

Response ParseReply(QNetworkReply* reply) {
  const int status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  // A SOAP fault comes with a 500, which isn't a failure to talk to it.
  const bool failed = reply->error() != QNetworkReply::NoError && status == 0;
  return Parse(status, reply->readAll(),
               failed ? reply->errorString() : QString());
}

}  // namespace UpnpSoap
