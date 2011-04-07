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

#ifndef LASTFMCOVERPROVIDER_H
#define LASTFMCOVERPROVIDER_H

#include "albumcoverfetcher.h"
#include "coverprovider.h"

#include <QObject>

class QNetworkReply;

// A built-in cover provider which fetches covers from last.fm.
class LastFmCoverProvider : public CoverProvider {
  Q_OBJECT
public:
  LastFmCoverProvider(QObject* parent);
  virtual ~LastFmCoverProvider() {}

  QNetworkReply* SendRequest(const QString& query);
  CoverSearchResults ParseReply(QNetworkReply* reply);
};

#endif // LASTFMCOVERPROVIDER_H
