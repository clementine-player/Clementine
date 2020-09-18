/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_DIGITALLY_DIGITALLYIMPORTEDCLIENT_H_
#define INTERNET_DIGITALLY_DIGITALLYIMPORTEDCLIENT_H_

#include <QDateTime>
#include <QObject>
#include <QSettings>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class DigitallyImportedClient : public QObject {
  Q_OBJECT

 public:
  explicit DigitallyImportedClient(const QString& service_name,
                                   QObject* parent = nullptr);

  static const char* kApiUsername;
  static const char* kApiPassword;
  static const char* kAuthUrl;
  static const char* kChannelListUrl;

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

  struct Channel {
    QUrl art_url_;

    QString director_;
    QString description_;
    QString name_;
    QString key_;

    bool operator<(const Channel& other) const { return name_ < other.name_; }
  };
  typedef QList<Channel> ChannelList;

  QNetworkReply* Auth(const QString& username, const QString& password);
  AuthReply ParseAuthReply(QNetworkReply* reply) const;

  QNetworkReply* GetChannelList();
  ChannelList ParseChannelList(QNetworkReply* reply) const;

 private:
  void SetAuthorisationHeader(QNetworkRequest* req) const;

 private:
  QNetworkAccessManager* network_;

  QString service_name_;
};

QDataStream& operator<<(QDataStream& out,
                        const DigitallyImportedClient::Channel& channel);
QDataStream& operator>>(QDataStream& in,
                        DigitallyImportedClient::Channel& channel);
Q_DECLARE_METATYPE(DigitallyImportedClient::Channel)

#endif  // INTERNET_DIGITALLY_DIGITALLYIMPORTEDCLIENT_H_
