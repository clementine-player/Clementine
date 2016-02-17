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

#ifndef ECHONESTIMAGES_H
#define ECHONESTIMAGES_H

#include <memory>

#include <QMultiMap>

#include <echonest/Artist.h>

#include "songinfo/songinfoprovider.h"

class NetworkAccessManager;
class QNetworkReply;

class EchoNestImages : public SongInfoProvider {
  Q_OBJECT

 public:
  EchoNestImages();
  virtual ~EchoNestImages();
  void FetchInfo(int id, const Song& metadata);

 private slots:
  void RequestFinished(QNetworkReply*, int id, Echonest::Artist artist);
  void IdsFound(QNetworkReply* reply, int id);

 private:
  void DoSpotifyImageRequest(const QString& id, int request_id);

  void RegisterReply(QNetworkReply* reply, int id);
  QMultiMap<int, QNetworkReply*> replies_;
  std::unique_ptr<NetworkAccessManager> network_;
};

#endif  // ECHONESTIMAGES_H
