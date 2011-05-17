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

#ifndef LASTFMCOVERPROVIDERFACTORY_H
#define LASTFMCOVERPROVIDERFACTORY_H

#include "coverproviderfactory.h"

#include <QObject>

class AlbumCoverFetcherSearch;
class CoverProvider;

class LastFmCoverProviderFactory : public CoverProviderFactory {
  Q_OBJECT

public:
  LastFmCoverProviderFactory(QObject* parent);
  ~LastFmCoverProviderFactory() {}

  CoverProvider* CreateCoverProvider(AlbumCoverFetcherSearch* parent);
};

#endif // LASTFMCOVERPROVIDERFACTORY_H
