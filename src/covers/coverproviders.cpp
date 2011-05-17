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

#include "config.h"
#include "coverproviderfactory.h"
#include "coverproviders.h"

#ifdef HAVE_LIBLASTFM
# include "lastfmcoverproviderfactory.h"
#endif

CoverProviders::CoverProviders()
{
   // registering built-in provider factories...

   // every built-in provider factory needs an explicit parent; otherwise,
   // the default parent, namely CoverProviders::instance(), will 
   // cause an infinite recursion here
#ifdef HAVE_LIBLASTFM
   cover_provider_factories_.append(new LastFmCoverProviderFactory(this));
#endif
}

void CoverProviders::AddProviderFactory(CoverProviderFactory* factory) {
  {
    QMutexLocker locker(&mutex_);
    Q_UNUSED(locker);

    cover_provider_factories_.append(factory);
    connect(factory, SIGNAL(destroyed()), SLOT(RemoveProviderFactory()));
  }
}

void CoverProviders::RemoveProviderFactory() {
  // qobject_cast doesn't work here with factories created by python
  CoverProviderFactory* factory = static_cast<CoverProviderFactory*>(sender());

  if (factory) {
    {
      QMutexLocker locker(&mutex_);
      Q_UNUSED(locker);

      cover_provider_factories_.removeAll(factory);
    }
  }
}

QList<CoverProvider*> CoverProviders::List(AlbumCoverFetcherSearch* parent) {
  {
    QMutexLocker locker(&mutex_);
    Q_UNUSED(locker);

    QList<CoverProvider*> result;
    foreach(CoverProviderFactory* factory, cover_provider_factories_) {
        result.append(factory->CreateCoverProvider(parent));
    }
    return result;
  }
}
