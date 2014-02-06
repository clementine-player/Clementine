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

#include "groovesharksearchprovider.h"

#include <QIcon>

#include "core/application.h"
#include "core/logging.h"
#include "covers/albumcoverloader.h"
#include "internet/groovesharkservice.h"

GroovesharkSearchProvider::GroovesharkSearchProvider(Application* app, QObject* parent)
    : SearchProvider(app, parent),
      service_(nullptr)
{
}

void GroovesharkSearchProvider::Init(GroovesharkService* service) {
  service_ = service;
  SearchProvider::Init("Grooveshark", "grooveshark",
                       QIcon(":providers/grooveshark.png"),
                       WantsDelayedQueries | ArtIsProbablyRemote | CanShowConfig);

  connect(service_, SIGNAL(SimpleSearchResults(int, SongList)),
          SLOT(SearchDone(int, SongList)));
  connect(service_, SIGNAL(AlbumSearchResult(int, QList<quint64>)),
          SLOT(AlbumSearchResult(int, QList<quint64>)));
  connect(service_, SIGNAL(AlbumSongsLoaded(quint64, SongList)),
          SLOT(AlbumSongsLoaded(quint64, SongList)));

  cover_loader_options_.desired_height_ = kArtHeight;
  cover_loader_options_.pad_output_image_ = true;
  cover_loader_options_.scale_output_image_ = true;

  connect(app_->album_cover_loader(),
          SIGNAL(ImageLoaded(quint64, QImage)),
          SLOT(AlbumArtLoaded(quint64, QImage)));
}

void GroovesharkSearchProvider::SearchAsync(int id, const QString& query) {
  const int service_id = service_->SimpleSearch(query);
  pending_searches_[service_id] = PendingState(id, TokenizeQuery(query));;

  const int album_id = service_->SearchAlbums(query);
  pending_searches_[album_id] = PendingState(id, TokenizeQuery(query));
}

void GroovesharkSearchProvider::SearchDone(int id, const SongList& songs) {
  // Map back to the original id.
  const PendingState state = pending_searches_.take(id);
  const int global_search_id = state.orig_id_;

  ResultList ret;
  foreach (const Song& song, songs) {
    Result result(this);
    result.metadata_ = song;

    ret << result;
  }

  emit ResultsAvailable(global_search_id, ret);
  MaybeSearchFinished(global_search_id);
}

void GroovesharkSearchProvider::AlbumSearchResult(int id, const QList<quint64>& albums_ids) {
  // Map back to the original id.
  const PendingState state = pending_searches_.take(id);
  const int global_search_id = state.orig_id_;
  if (albums_ids.isEmpty()) {
    MaybeSearchFinished(global_search_id);
    return;
  }
  foreach (const quint64 album_id, albums_ids) {
    pending_searches_[album_id] = PendingState(global_search_id, QStringList());
  }

}

void GroovesharkSearchProvider::MaybeSearchFinished(int id) {
  if (pending_searches_.keys(PendingState(id, QStringList())).isEmpty()) {
    emit SearchFinished(id);
  }
}


void GroovesharkSearchProvider::LoadArtAsync(int id, const Result& result) {
  quint64 loader_id = app_->album_cover_loader()->LoadImageAsync(
        cover_loader_options_, result.metadata_);
  cover_loader_tasks_[loader_id] = id;
}

void GroovesharkSearchProvider::AlbumArtLoaded(quint64 id, const QImage& image) {
  if (!cover_loader_tasks_.contains(id)) {
    return;
  }
  int original_id = cover_loader_tasks_.take(id);
  emit ArtLoaded(original_id, image);
}

bool GroovesharkSearchProvider::IsLoggedIn() {
  return (service_ && service_->IsLoggedIn());
}

void GroovesharkSearchProvider::ShowConfig() {
  service_->ShowConfig();
}

void GroovesharkSearchProvider::AlbumSongsLoaded(quint64 id, const SongList& songs) {
  const PendingState state = pending_searches_.take(id);
  const int global_search_id = state.orig_id_;
  ResultList ret;
  foreach (const Song& s, songs) {
    Result result(this);
    result.metadata_ = s;
    ret << result;
  }

  emit ResultsAvailable(global_search_id, ret);
  MaybeSearchFinished(global_search_id);
}
