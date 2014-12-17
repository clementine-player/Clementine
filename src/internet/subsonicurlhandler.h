/* This file is part of Clementine.
   Copyright 2012-2013, Alan Briolat <alan.briolat@gmail.com>
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

#ifndef INTERNET_SUBSONICURLHANDLER_H_
#define INTERNET_SUBSONICURLHANDLER_H_

#include "core/urlhandler.h"

class SubsonicService;

// Subsonic URL handler: subsonic://id
class SubsonicUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  SubsonicUrlHandler(SubsonicService* service, QObject* parent);

  QString scheme() const { return "subsonic"; }
  QIcon icon() const { return QIcon(":providers/subsonic-32.png"); }
  LoadResult StartLoading(const QUrl& url);
  // LoadResult LoadNext(const QUrl& url);

 private:
  SubsonicService* service_;
};

#endif  // INTERNET_SUBSONICURLHANDLER_H_
