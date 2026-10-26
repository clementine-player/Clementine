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

#ifndef UPNP_UPNPSOAP_H_
#define UPNP_UPNPSOAP_H_

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

// UPnP control requests: a SOAP action on one of a device's services, eg.
// AVTransport's Play.
namespace UpnpSoap {

// Arguments in the order the service describes them, which devices expect.
typedef QList<QPair<QString, QString>> Args;

struct Action {
  // The service's controlURL, from the device description.
  QUrl control_url;
  // eg. "urn:schemas-upnp-org:service:AVTransport:1".
  QString service_type;
  QString name;
  Args args;
};

// What a device answered.
struct Response {
  bool ok = false;
  // 0 if the device couldn't be reached.
  int http_status = 0;
  // The response's out arguments, by name, unescaped.
  QMap<QString, QString> values;
  // When !ok: the UPnP error code from a SOAP fault, or 0 if the request
  // failed some other way.
  int error_code = 0;
  QString error;
};

QByteArray Envelope(const Action& action);
// |http_status| is 0 if the request didn't get that far; |network_error| says
// why.
Response Parse(int http_status, const QByteArray& body,
               const QString& network_error = QString());

// Sends |action|. The caller owns the reply, and gives it to ParseReply once
// it's finished.
QNetworkReply* Send(QNetworkAccessManager* network, const Action& action);
Response ParseReply(QNetworkReply* reply);

}  // namespace UpnpSoap

#endif  // UPNP_UPNPSOAP_H_
