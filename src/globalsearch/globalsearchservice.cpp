/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "globalsearch.h"
#include "globalsearchservice.h"
#include "globalsearch/globalsearchadaptor.h"
#include "core/logging.h"
#include "core/mpris_common.h"


GlobalSearchService::GlobalSearchService(GlobalSearch* engine, QObject* parent)
  : QObject(parent),
    engine_(engine)
{
  qDBusRegisterMetaType<GlobalSearchServiceResult>();
  qDBusRegisterMetaType<GlobalSearchServiceResultList>();

  new GlobalSearchAdaptor(this);
  QDBusConnection::sessionBus().registerObject("/GlobalSearch", this);

  connect(engine_, SIGNAL(ResultsAvailable(int,SearchProvider::ResultList)),
          this, SLOT(ResultsAvailableSlot(int,SearchProvider::ResultList)),
          Qt::QueuedConnection);
  connect(engine_, SIGNAL(SearchFinished(int)),
          this, SLOT(SearchFinishedSlot(int)),
          Qt::QueuedConnection);
  connect(engine_, SIGNAL(ArtLoaded(int,QPixmap)),
          this, SLOT(ArtLoadedSlot(int,QPixmap)),
          Qt::QueuedConnection);
}

int GlobalSearchService::StartSearch(const QString& query, bool prefetch_art) {
  PendingSearch pending_search;
  pending_search.prefetch_art_ = prefetch_art;

  const int id = engine_->SearchAsync(query);
  pending_searches_[id] = pending_search;
  return id;
}

void GlobalSearchService::CancelSearch(int id) {
  if (!pending_searches_.contains(id))
    return;

  engine_->CancelSearch(id);
  pending_searches_.remove(id);
}

void GlobalSearchService::ResultsAvailableSlot(int id, const SearchProvider::ResultList& results) {
  if (!pending_searches_.contains(id))
    return;

  const PendingSearch& pending = pending_searches_[id];

  GlobalSearchServiceResultList ret;
  foreach (const SearchProvider::Result& result, results) {
    const int result_id = next_result_id_ ++;

    RecentResult& recent = recent_results_[result_id];
    recent.result_.art_on_the_way_ = false;

    // Prefetch art if it was requested
    if (pending.prefetch_art_ && !result.provider_->art_is_probably_remote()) {
      const int art_id = engine_->LoadArtAsync(result);
      prefetching_art_[art_id] = result_id;
      recent.result_.art_on_the_way_ = true;
    }

    // Build the result to send back
    recent.result_.result_id_ = result_id;
    recent.result_.provider_name_ = result.provider_->name();
    recent.result_.type_ = result.type_;
    recent.result_.match_quality_ = result.match_quality_;

    recent.result_.album_size_ = result.album_size_;

    recent.result_.title_ = result.metadata_.title();
    recent.result_.artist_ = result.metadata_.artist();
    recent.result_.album_ = result.metadata_.album();
    recent.result_.album_artist_ = result.metadata_.albumartist();
    recent.result_.is_compilation_ = result.metadata_.is_compilation();
    recent.result_.track_ = result.metadata_.track();

    ret << recent.result_;
  }

  emit ResultsAvailable(id, ret);
}

void GlobalSearchService::SearchFinishedSlot(int id) {
  if (!pending_searches_.contains(id))
    return;

  emit SearchFinished(id);
  pending_searches_.remove(id);
}

void GlobalSearchService::ArtLoadedSlot(int id, const QPixmap& pixmap) {
  QMap<int, int>::iterator it = prefetching_art_.find(id);
  if (it == prefetching_art_.end())
    return;

  const int result_id = prefetching_art_.take(id);
  QMap<int, RecentResult>::iterator it2 = recent_results_.find(result_id);
  if (it2 == recent_results_.end())
    return;

  // Encode the pixmap as a png
  QBuffer buf;
  buf.open(QIODevice::WriteOnly);
  pixmap.toImage().save(&buf, "PNG");
  buf.close();

  emit ArtLoaded(result_id, buf.data());
}
