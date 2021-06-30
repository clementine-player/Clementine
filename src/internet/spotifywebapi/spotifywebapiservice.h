/* This file is part of Clementine.
   Copyright 2021, Kenman Tsang <kentsangkm@pm.me>

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

#ifndef SPOTIFYWEBAPISERVICE_H
#define SPOTIFYWEBAPISERVICE_H

#include <chrono>

#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"

class NetworkAccessManager;

class SpotifyWebApiService : public InternetService {
  Q_OBJECT

 public:
  SpotifyWebApiService(Application* app, InternetModel* parent);
  ~SpotifyWebApiService();

  QStandardItem* CreateRootItem() override;
  void LazyPopulate(QStandardItem* parent) override;

 public:
  void Search(int searchId, QString queryStr);

 private:
  void OnTokenReady(const QJsonDocument&, int searchId, QString queryStr);
  void OnReadyToSearch(int searchId, QString queryStr);
  void BuildResultList(const QJsonDocument&, int searchId);

 signals:
  void SearchFinished(int searchId, const QList<Song>&);

 private:
  QJsonDocument ParseJsonReplyWithGzip(QNetworkReply* reply);

 private:
  QStandardItem* root_;
  NetworkAccessManager* network_;

  QString token_;
  qint64 token_expiration_ms_;
};

#endif  // SPOTIFYWEBAPISERVICE_H
