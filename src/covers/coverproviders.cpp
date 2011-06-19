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
#include "coverprovider.h"
#include "coverproviders.h"
#include "core/logging.h"

CoverProviders::CoverProviders()
  : QObject(NULL) {
}

void CoverProviders::AddProvider(CoverProvider* provider) {
  {
    QMutexLocker locker(&mutex_);
    cover_providers_.append(provider);
  }

  qLog(Debug) << "Registered cover provider" << provider->name();

  connect(provider, SIGNAL(destroyed()), SLOT(ProviderDestroyed()));
}

void CoverProviders::RemoveProvider(CoverProvider* provider) {
  if (!provider)
    return;

  qLog(Debug) << "Unregistered cover provider" << provider->name();

  {
    QMutexLocker locker(&mutex_);
    cover_providers_.removeAll(provider);
  }
}

void CoverProviders::ProviderDestroyed() {
  CoverProvider* provider = static_cast<CoverProvider*>(sender());
  RemoveProvider(provider);
}

int CoverProviders::NextId() {
  return next_id_.fetchAndAddRelaxed(1);
}
