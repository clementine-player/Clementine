/* This file is part of Clementine.
   Copyright 2012-2013, David Sansome <me@davidsansome.com>
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

#include "lastfmcompat.h"
#include "core/logging.h"

namespace lastfm {
namespace compat {

XmlQuery EmptyXmlQuery() { return XmlQuery(); }

bool ParseQuery(const QByteArray& data, XmlQuery* query,
                bool* connection_problems) {
  const bool ret = query->parse(data);

  if (connection_problems) {
    *connection_problems =
        !ret &&
        query->parseError().enumValue() == lastfm::ws::MalformedResponse;
  }

  return ret;
}

bool ParseUserList(QNetworkReply* reply, QList<User>* users) {
  lastfm::XmlQuery lfm;
  if (!lfm.parse(reply->readAll())) {
    return false;
  }

  *users = lastfm::UserList(lfm).users();
  return true;
}

uint ScrobbleTimeMin() { return lastfm::ScrobblePoint::scrobbleTimeMin(); }

}  // namespace compat
}  // namespace lastfm
