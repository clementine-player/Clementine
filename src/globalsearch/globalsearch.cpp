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

#include <QStringBuilder>
#include <QUrl>

const int GlobalSearch::kDelayedSearchTimeoutMs = 200;


GlobalSearch::GlobalSearch(QObject* parent)
  : QObject(parent),
    next_id_(1)
{
}

void GlobalSearch::AddProvider(SearchProvider* provider) {
  Q_ASSERT(!provider->name().isEmpty());

  connect(provider, SIGNAL(ResultsAvailable(int,SearchProvider::ResultList)),
          SLOT(ResultsAvailableSlot(int,SearchProvider::ResultList)));
  connect(provider, SIGNAL(SearchFinished(int)),
          SLOT(SearchFinishedSlot(int)));
  connect(provider, SIGNAL(ArtLoaded(int,QImage)),
          SLOT(ArtLoadedSlot(int,QImage)));
  connect(provider, SIGNAL(TracksLoaded(int,MimeData*)),
          SIGNAL(TracksLoaded(int,MimeData*)));
  connect(provider, SIGNAL(destroyed(QObject*)),
          SLOT(ProviderDestroyedSlot(QObject*)));

  providers_ << provider;
}

int GlobalSearch::SearchAsync(const QString& query) {
  const int id = next_id_ ++;

  int timer_id = -1;

  pending_search_providers_[id] = providers_.count();
  foreach (SearchProvider* provider, providers_) {
    if (provider->wants_delayed_queries()) {
      if (timer_id == -1) {
        timer_id = startTimer(kDelayedSearchTimeoutMs);
        delayed_searches_[timer_id].id_ = id;
        delayed_searches_[timer_id].query_ = query;
      }
      delayed_searches_[timer_id].providers_ << provider;
    } else {
      provider->SearchAsync(id, query);
    }
  }

  return id;
}

void GlobalSearch::CancelSearch(int id) {
  QMap<int, DelayedSearch>::iterator it;
  for (it = delayed_searches_.begin() ; it != delayed_searches_.end() ; ++it) {
    if (it.value().id_ == id) {
      killTimer(it.key());
      delayed_searches_.erase(it);
      return;
    }
  }
}

void GlobalSearch::timerEvent(QTimerEvent* e) {
  QMap<int, DelayedSearch>::iterator it = delayed_searches_.find(e->timerId());
  if (it != delayed_searches_.end()) {
    foreach (SearchProvider* provider, it.value().providers_) {
      provider->SearchAsync(it.value().id_, it.value().query_);
    }
    delayed_searches_.erase(it);
    return;
  }

  QObject::timerEvent(e);
}

QString GlobalSearch::PixmapCacheKey(const SearchProvider::Result& result) const {
  return QString::number(qulonglong(result.provider_))
       % "," % QString::number(int(result.type_))
       % "," % result.metadata_.url().toString();
}

void GlobalSearch::ResultsAvailableSlot(int id, SearchProvider::ResultList results) {
  if (results.isEmpty())
    return;

  // Load cached pixmaps into the results
  for (SearchProvider::ResultList::iterator it = results.begin() ; it != results.end() ; ++it) {
    it->pixmap_cache_key_ = PixmapCacheKey(*it);
  }

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

  pending_art_searches_[id] = result.pixmap_cache_key_;

  if (result.provider_->wants_serialised_art()) {
    QueuedArt request;
    request.id_ = id;
    request.result_ = result;

    queued_art_[result.provider_].append(request);

    if (queued_art_[result.provider_].count() == 1) {
      TakeNextQueuedArt(result.provider_);
    }
  } else {
    result.provider_->LoadArtAsync(id, result);
  }

  return id;
}

void GlobalSearch::TakeNextQueuedArt(SearchProvider* provider) {
  if (queued_art_[provider].isEmpty())
    return;

  const QueuedArt& data = queued_art_[provider].first();
  provider->LoadArtAsync(data.id_, data.result_);
}

void GlobalSearch::ArtLoadedSlot(int id, const QImage& image) {
  SearchProvider* provider = static_cast<SearchProvider*>(sender());
  const QString key = pending_art_searches_.take(id);

  QPixmap pixmap = QPixmap::fromImage(image);
  pixmap_cache_.insert(key, pixmap);

  emit ArtLoaded(id, pixmap);

  if (!queued_art_[provider].isEmpty()) {
    queued_art_[provider].removeFirst();
    TakeNextQueuedArt(provider);
  }
}

bool GlobalSearch::FindCachedPixmap(const SearchProvider::Result& result,
                                    QPixmap* pixmap) const {
  return pixmap_cache_.find(result.pixmap_cache_key_, pixmap);
}

int GlobalSearch::LoadTracksAsync(const SearchProvider::Result& result) {
  const int id = next_id_ ++;

  result.provider_->LoadTracksAsync(id, result);

  return id;
}

