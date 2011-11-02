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

#ifndef DIGITALLYIMPORTEDCLIENT_H
#define DIGITALLYIMPORTEDCLIENT_H

#include <QDateTime>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class DigitallyImportedClient : public QObject {
  Q_OBJECT

public:
  DigitallyImportedClient(const QString& service_name, QObject* parent = 0);

  static const char* kApiUsername;
  static const char* kApiPassword;
  static const char* kAuthUrl;

  struct AuthReply {
    bool success_;

    // Set if success_ == false
    QString error_reason_;

    // Set if success_ == true
    QString first_name_;
    QString last_name_;
    QDateTime expires_;
    QString listen_hash_;
  };

  QNetworkReply* Auth(const QString& username, const QString& password);
  AuthReply ParseAuthReply(QNetworkReply* reply) const;

private:
  QNetworkAccessManager* network_;

  QString service_name_;
};

#endif // DIGITALLYIMPORTEDCLIENT_H
