/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_LASTFMURLHANDLER_H_
#define INTERNET_LASTFMURLHANDLER_H_

#include "core/urlhandler.h"

class LastFMService;

class LastFMUrlHandler : public UrlHandler {
  friend class LastFMService;

 public:
  LastFMUrlHandler(LastFMService* service, QObject* parent);

  QString scheme() const { return "lastfm"; }
  QIcon icon() const { return QIcon(":last.fm/as.png"); }
  LoadResult StartLoading(const QUrl& url);
  LoadResult LoadNext(const QUrl& url);

  void TunerTrackAvailable();
  void TunerError();

 private:
  LastFMService* service_;
};

#endif  // INTERNET_LASTFMURLHANDLER_H_
