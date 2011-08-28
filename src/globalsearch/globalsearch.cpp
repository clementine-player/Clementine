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

#include "librarysearchprovider.h"
#include "globalsearch.h"
#include "core/logging.h"

GlobalSearch::GlobalSearch(QObject* parent)
  : QObject(parent),
    next_id_(1)
{
}

void GlobalSearch::AddProvider(SearchProvider* provider) {
  connect(provider, SIGNAL(ResultsAvailable(int,SearchProvider::ResultList)),
          SLOT(ResultsAvailableSlot(int,SearchProvider::ResultList)));
  connect(provider, SIGNAL(SearchFinished(int)),
          SLOT(SearchFinishedSlot(int)));
  connect(provider, SIGNAL(ArtLoaded(int,QImage)), SIGNAL(ArtLoaded(int,QImage)));
  connect(provider, SIGNAL(destroyed(QObject*)),
          SLOT(ProviderDestroyedSlot(QObject*)));

  providers_ << provider;
}

int GlobalSearch::SearchAsync(const QString& query) {
  const int id = next_id_ ++;

  pending_search_providers_[id] = providers_.count();
  foreach (SearchProvider* provider, providers_) {
    provider->SearchAsync(id, query);
  }

  return id;
}

void GlobalSearch::ResultsAvailableSlot(int id, const SearchProvider::ResultList& results) {
  if (!results.isEmpty())
    emit ResultsAvailable(id, results);
}

void GlobalSearch::SearchFinishedSlot(int id) {
  if (!pending_search_providers_.contains(id))
    return;

  SearchProvider* provider = static_cast<SearchProvider*>(sender());
  const int remaining = --pending_search_providers_[id];

  emit ProviderSearchFinished(id, provider);
  if (remaining == 0) {
    emit SearchFinished(id);
    pending_search_providers_.remove(id);
  }
}

void GlobalSearch::ProviderDestroyedSlot(QObject* object) {
  SearchProvider* provider = static_cast<SearchProvider*>(object);
  if (!providers_.contains(provider))
    return;

  providers_.removeAll(provider);
  emit ProviderDestroyed(provider);

  // We have to abort any pending searches since we can't tell whether they
  // were on this provider.
  foreach (int id, pending_search_providers_.keys()) {
    emit SearchFinished(id);
  }
  pending_search_providers_.clear();
}

int GlobalSearch::LoadArtAsync(const SearchProvider::Result& result) {
  const int id = next_id_ ++;
  result.provider_->LoadArtAsync(id, result);
  return id;
}

