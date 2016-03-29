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
#include "urlsearchprovider.h"
#include "core/application.h"
#include "core/logging.h"
#include "covers/albumcoverloader.h"

#include <QSettings>
#include <QStringBuilder>
#include <QTimerEvent>
#include <QUrl>

#include <algorithm>

const int GlobalSearch::kDelayedSearchTimeoutMs = 200;
const char* GlobalSearch::kSettingsGroup = "GlobalSearch";
const int GlobalSearch::kMaxResultsPerEmission = 500;

GlobalSearch::GlobalSearch(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      next_id_(1),
      url_provider_(new UrlSearchProvider(app, this)) {
  cover_loader_options_.desired_height_ = SearchProvider::kArtHeight;
  cover_loader_options_.pad_output_image_ = true;
  cover_loader_options_.scale_output_image_ = true;

  connect(app_->album_cover_loader(), SIGNAL(ImageLoaded(quint64, QImage)),
          SLOT(AlbumArtLoaded(quint64, QImage)));
  connect(this, SIGNAL(SearchAsyncSig(int,QString)),
          this, SLOT(DoSearchAsync(int,QString)));

  ConnectProvider(url_provider_);
}

void GlobalSearch::ConnectProvider(SearchProvider* provider) {
  connect(provider, SIGNAL(ResultsAvailable(int, SearchProvider::ResultList)),
          SLOT(ResultsAvailableSlot(int, SearchProvider::ResultList)));
  connect(provider, SIGNAL(SearchFinished(int)), SLOT(SearchFinishedSlot(int)));
  connect(provider, SIGNAL(ArtLoaded(int, QImage)),
          SLOT(ArtLoadedSlot(int, QImage)));
  connect(provider, SIGNAL(destroyed(QObject*)),
          SLOT(ProviderDestroyedSlot(QObject*)));
}

void GlobalSearch::AddProvider(SearchProvider* provider) {
  Q_ASSERT(!provider->name().isEmpty());

  bool enabled = provider->is_enabled_by_default();

  // Check if there is saved enabled/disabled state for this provider.
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QVariant enabled_variant = s.value("enabled_" + provider->id());
  if (enabled_variant.isValid()) {
    enabled = enabled_variant.toBool();
  }

  // Add data
  ProviderData data;
  data.enabled_ = enabled;
  providers_[provider] = data;

  ConnectProvider(provider);
  emit ProviderAdded(provider);
}

int GlobalSearch::SearchAsync(const QString& query) {
  const int id = next_id_++;

  emit SearchAsyncSig(id, query);

  return id;
}

void GlobalSearch::DoSearchAsync(int id, const QString& query) {
  pending_search_providers_[id] = 0;

  int timer_id = -1;

  if (url_provider_->LooksLikeUrl(query)) {
    url_provider_->SearchAsync(id, query);
  } else {
    for (SearchProvider* provider : providers_.keys()) {
      if (!is_provider_usable(provider)) continue;

      pending_search_providers_[id]++;

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
  }
}

void GlobalSearch::CancelSearch(int id) {
  QMap<int, DelayedSearch>::iterator it;
  for (it = delayed_searches_.begin(); it != delayed_searches_.end(); ++it) {
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
    for (SearchProvider* provider : it.value().providers_) {
      provider->SearchAsync(it.value().id_, it.value().query_);
    }
    delayed_searches_.erase(it);
    return;
  }

  QObject::timerEvent(e);
}

QString GlobalSearch::PixmapCacheKey(const SearchProvider::Result& result)
    const {
  return "globalsearch:" % QString::number(qulonglong(result.provider_)) % "," %
         result.metadata_.url().toString();
}

void GlobalSearch::ResultsAvailableSlot(int id,
                                        SearchProvider::ResultList results) {
  if (results.isEmpty()) return;

  // Limit the number of results that are used from each emission.
  // Just a sanity check to stop some providers (Jamendo) returning thousands
  // of results.
  if (results.count() > kMaxResultsPerEmission) {
    SearchProvider::ResultList::iterator begin = results.begin();
    std::advance(begin, kMaxResultsPerEmission);
    results.erase(begin, results.end());
  }

  // Load cached pixmaps into the results
  for (SearchProvider::ResultList::iterator it = results.begin();
       it != results.end(); ++it) {
    it->pixmap_cache_key_ = PixmapCacheKey(*it);
  }

  emit ResultsAvailable(id, results);
}

void GlobalSearch::SearchFinishedSlot(int id) {
  if (!pending_search_providers_.contains(id)) return;

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
  if (!providers_.contains(provider)) return;

  providers_.remove(provider);
  emit ProviderRemoved(provider);

  // We have to abort any pending searches since we can't tell whether they
  // were on this provider.
  for (int id : pending_search_providers_.keys()) {
    emit SearchFinished(id);
  }
  pending_search_providers_.clear();
}

QList<SearchProvider*> GlobalSearch::providers() const {
  return providers_.keys();
}

int GlobalSearch::LoadArtAsync(const SearchProvider::Result& result) {
  const int id = next_id_++;

  pending_art_searches_[id] = result.pixmap_cache_key_;

  if (providers_.contains(result.provider_) &&
      !is_provider_usable(result.provider_)) {
    emit ArtLoaded(id, QPixmap());
    return id;
  }

  if (result.provider_->art_is_in_song_metadata()) {
    quint64 loader_id = app_->album_cover_loader()->LoadImageAsync(
        cover_loader_options_, result.metadata_);
    cover_loader_tasks_[loader_id] = id;
  } else if (providers_.contains(result.provider_) &&
             result.provider_->wants_serialised_art()) {
    QueuedArt request;
    request.id_ = id;
    request.result_ = result;

    QList<QueuedArt>* queued_art = &providers_[result.provider_].queued_art_;

    queued_art->append(request);

    if (queued_art->count() == 1) {
      TakeNextQueuedArt(result.provider_);
    }
  } else {
    result.provider_->LoadArtAsync(id, result);
  }

  return id;
}

void GlobalSearch::TakeNextQueuedArt(SearchProvider* provider) {
  if (!providers_.contains(provider) ||
      providers_[provider].queued_art_.isEmpty())
    return;

  const QueuedArt& data = providers_[provider].queued_art_.first();
  provider->LoadArtAsync(data.id_, data.result_);
}

void GlobalSearch::ArtLoadedSlot(int id, const QImage& image) {
  SearchProvider* provider = static_cast<SearchProvider*>(sender());
  HandleLoadedArt(id, image, provider);
}

void GlobalSearch::AlbumArtLoaded(quint64 id, const QImage& image) {
  if (!cover_loader_tasks_.contains(id)) return;
  int orig_id = cover_loader_tasks_.take(id);

  HandleLoadedArt(orig_id, image, nullptr);
}

void GlobalSearch::HandleLoadedArt(int id, const QImage& image,
                                   SearchProvider* provider) {
  const QString key = pending_art_searches_.take(id);

  QPixmap pixmap = QPixmap::fromImage(image);
  pixmap_cache_.insert(key, pixmap);

  emit ArtLoaded(id, pixmap);

  if (provider && providers_.contains(provider) &&
      !providers_[provider].queued_art_.isEmpty()) {
    providers_[provider].queued_art_.removeFirst();
    TakeNextQueuedArt(provider);
  }
}

bool GlobalSearch::FindCachedPixmap(const SearchProvider::Result& result,
                                    QPixmap* pixmap) const {
  return pixmap_cache_.find(result.pixmap_cache_key_, pixmap);
}

MimeData* GlobalSearch::LoadTracks(const SearchProvider::ResultList& results) {
  // Different providers might create MimeData in different ways, so it's not
  // possible to combine different providers.  Just take the results from a
  // single provider.
  if (results.isEmpty()) {
    return nullptr;
  }

  SearchProvider* first_provider = results[0].provider_;
  SearchProvider::ResultList results_copy;
  for (const SearchProvider::Result& result : results) {
    if (result.provider_ == first_provider) {
      results_copy << result;
    }
  }

  return first_provider->LoadTracks(results);
}

bool GlobalSearch::SetProviderEnabled(const SearchProvider* const_provider,
                                      bool enabled) {
  SearchProvider* provider = const_cast<SearchProvider*>(const_provider);
  if (!providers_.contains(provider)) return true;

  if (providers_[provider].enabled_ != enabled) {
    // If we try to enable this provider but it is not logged in, don't change
    // state, and show configuration menu, if any
    if (enabled && !provider->IsLoggedIn()) {
      provider->ShowConfig();
      return false;
    } else {
      providers_[provider].enabled_ = enabled;
      emit ProviderToggled(provider, enabled);
      SaveProvidersSettings();
      return true;
    }
  }
  return true;
}

bool GlobalSearch::is_provider_enabled(const SearchProvider* const_provider)
    const {
  SearchProvider* provider = const_cast<SearchProvider*>(const_provider);

  if (!providers_.contains(provider)) return false;
  return providers_[provider].enabled_;
}

bool GlobalSearch::is_provider_usable(SearchProvider* provider) const {
  return is_provider_enabled(provider) && provider->IsLoggedIn();
}

void GlobalSearch::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  for (SearchProvider* provider : providers_.keys()) {
    QVariant value = s.value("enabled_" + provider->id());
    if (!value.isValid()) continue;
    const bool enabled = value.toBool();

    if (enabled != providers_[provider].enabled_) {
      providers_[provider].enabled_ = enabled;
      emit ProviderToggled(provider, enabled);
    }
  }
}

void GlobalSearch::SaveProvidersSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  for (SearchProvider* provider : providers_.keys()) {
    s.setValue("enabled_" + provider->id(), providers_[provider].enabled_);
  }
}

QStringList GlobalSearch::GetSuggestions(int count) {
  QStringList ret;

  // Get count suggestions from each provider
  for (SearchProvider* provider : providers_.keys()) {
    if (is_provider_enabled(provider) && provider->can_give_suggestions()) {
      for (QString suggestion : provider->GetSuggestions(count)) {
        suggestion = suggestion.trimmed().toLower();

        if (!suggestion.isEmpty()) {
          ret << suggestion;
        }
      }
    }
  }

  // Randomize the suggestions
  std::random_shuffle(ret.begin(), ret.end());

  // Only return the first count
  while (ret.length() > count) {
    ret.removeLast();
  }
  return ret;
}
