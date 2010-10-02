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

#include "artistinfoprovider.h"

#include <boost/shared_ptr.hpp>

class QNetworkReply;

class EchoNestArtistInfo : public ArtistInfoProvider {
  Q_OBJECT

public:
  EchoNestArtistInfo(QObject* parent = 0);

  void FetchInfo(int id, const QString& artist);

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
};

#endif // ECHONESTARTISTINFO_H
