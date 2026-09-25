/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_PLEX_PLEXURLHANDLER_H_
#define INTERNET_PLEX_PLEXURLHANDLER_H_

#include "core/urlhandler.h"

class PlexService;

// Turns plex://<machine_id>/<rating_key>?part=... into a stream URL on the
// connected server.
class PlexUrlHandler : public UrlHandler {
  Q_OBJECT

 public:
  PlexUrlHandler(PlexService* service, QObject* parent);

  QString scheme() const override;
  QIcon icon() const override;
  LoadResult StartLoading(const QUrl& url) override;

 private:
  PlexService* service_;
};

#endif  // INTERNET_PLEX_PLEXURLHANDLER_H_
