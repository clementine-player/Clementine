/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_LASTFM_LASTFMCOMPAT_H_
#define INTERNET_LASTFM_LASTFMCOMPAT_H_

#include "config.h"
#include "fixlastfm.h"

#ifdef HAVE_LIBLASTFM1
#include <lastfm/Audioscrobbler.h>
#include <lastfm/misc.h>
#include <lastfm/ScrobbleCache.h>
#include <lastfm/ScrobblePoint.h>
#include <lastfm/User.h>
#include <lastfm/ws.h>
#include <lastfm/XmlQuery.h>
#else
#include <lastfm/Audioscrobbler>
#include <lastfm/misc.h>
#include <lastfm/ScrobbleCache>
#include <lastfm/ScrobblePoint>
#include <lastfm/User>
#include <lastfm/ws.h>
#include <lastfm/XmlQuery>
#endif

namespace lastfm {
namespace compat {

lastfm::XmlQuery EmptyXmlQuery();

bool ParseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                bool* connection_problems = nullptr);

bool ParseUserList(QNetworkReply* reply, QList<lastfm::User>* users);

uint ScrobbleTimeMin();

#ifdef HAVE_LIBLASTFM1
typedef lastfm::ScrobbleCache ScrobbleCache;
typedef lastfm::User AuthenticatedUser;
#else
typedef ::ScrobbleCache ScrobbleCache;
typedef lastfm::AuthenticatedUser AuthenticatedUser;
#endif
}  // namespace compat
}  // namespace lastfm

#endif  // INTERNET_LASTFM_LASTFMCOMPAT_H_
