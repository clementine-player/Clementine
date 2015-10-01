/* This file is part of Clementine.
   Copyright 2015, John Maguire <john.maguire@gmail.com>

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

#ifndef RECORDINGNETWORKACCESSMANAGER_H
#define RECORDINGNETWORKACCESSMANAGER_H

#include <QMap>
#include <QMutex>
#include <QNetworkAccessManager>

class RecordingNetworkAccessManager : public QNetworkAccessManager {
  Q_OBJECT
 public:
  explicit RecordingNetworkAccessManager(QObject* parent = nullptr);

  struct NetworkStat {
    QString url;  // Stripped of params.
    QNetworkAccessManager::Operation operation;
    int bytes_received;

    NetworkStat(QString, QNetworkAccessManager::Operation, int);
  };

  static QList<NetworkStat*> GetNetworkStatistics();

 protected:
  QNetworkReply* createRequest(
      Operation op, const QNetworkRequest& req, QIODevice* data = 0) override;

 private slots:
  void Finished(QNetworkReply* reply) const;

 private:
  static void RecordRequest(const QNetworkReply& reply);
  static void StaticInit();

  static QMutex sMutex;
  static QList<NetworkStat*>* sTrafficStats;
};

#endif  // RECORDINGNETWORKACCESSMANAGER_H
