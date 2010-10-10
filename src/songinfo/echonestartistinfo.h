/* This file is part of Clementine.

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

#ifndef ECHONESTARTISTINFO_H
#define ECHONESTARTISTINFO_H

#include "songinfoprovider.h"

#include <QMap>

#include <boost/shared_ptr.hpp>

class QNetworkReply;

class EchoNestArtistInfo : public SongInfoProvider {
  Q_OBJECT

public:
  EchoNestArtistInfo();

  void FetchInfo(int id, const Song& metadata);

private slots:
  void BiographiesFinished();
  void ImagesFinished();

private:
  struct Request;
  typedef boost::shared_ptr<Request> RequestPtr;

  void ConnectReply(RequestPtr request, QNetworkReply* reply, const char* slot);
  RequestPtr ReplyFinished(QNetworkReply* reply);

private:
  QList<RequestPtr> requests_;
  QMap<QString, int> site_relevance_;
  QMap<QString, QIcon> site_icons_;
};

#endif // ECHONESTARTISTINFO_H
