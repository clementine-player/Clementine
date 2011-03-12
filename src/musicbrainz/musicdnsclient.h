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

#ifndef MUSICDNSCLIENT_H
#define MUSICDNSCLIENT_H

#include <QMap>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class MusicDnsClient : public QObject {
  Q_OBJECT

public:
  MusicDnsClient(QObject* parent = 0);

  void Start(int id, const QString& fingerprint, int duration_msec);
  void Cancel(int id);
  void CancelAll();

signals:
  void Finished(int id, const QString& puid);

private slots:
  void RequestFinished();

private:
  static const char* kClientId;
  static const char* kUrl;

  QNetworkAccessManager* network_;
  QMap<QNetworkReply*, int> requests_;
};

#endif // MUSICDNSCLIENT_H
