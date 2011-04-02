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

#include "coverprovider.h"
#include "coverproviders.h"
#include "lastfmcoverprovider.h"

CoverProviders::CoverProviders()
{
   // registering built-in providers...

   // every built-in provider needs an explicit parent; otherwise,
   // the default parent, namely CoverProviders::instance(), will 
   // cause an infinite recursion here
   cover_providers_.append(new LastFmCoverProvider(this));
}

void CoverProviders::AddCoverProvider(CoverProvider* provider) {
  {
    QMutexLocker locker(&mutex_);
    Q_UNUSED(locker);

    cover_providers_.append(provider);
    connect(provider, SIGNAL(destroyed()), SLOT(RemoveCoverProvider()));
  }
}

void CoverProviders::RemoveCoverProvider() {
  // qobject_cast doesn't work here with providers created by python
  CoverProvider* provider = static_cast<CoverProvider*>(sender());

  if (provider) {
    {
      QMutexLocker locker(&mutex_);
      Q_UNUSED(locker);

      cover_providers_.removeAll(provider);
    }
  }
}

const QList<CoverProvider*> CoverProviders::List() {
  {
    QMutexLocker locker(&mutex_);
    Q_UNUSED(locker);

    return QList<CoverProvider*>(cover_providers_);
  }
}
