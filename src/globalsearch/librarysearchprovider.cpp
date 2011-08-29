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
#include "core/logging.h"
#include "covers/albumcoverloader.h"
#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "library/sqlrow.h"
#include "playlist/songmimedata.h"


LibrarySearchProvider::LibrarySearchProvider(LibraryBackendInterface* backend,
                                             const QString& name,
                                             const QIcon& icon,
                                             QObject* parent)
  : BlockingSearchProvider(parent),
    backend_(backend),
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this))
{
  Init(name, icon, false, true);

  cover_loader_->Start(true);
  cover_loader_->Worker()->SetDesiredHeight(kArtHeight);
  cover_loader_->Worker()->SetPadOutputImage(true);
  cover_loader_->Worker()->SetScaleOutputImage(true);

  connect(cover_loader_->Worker().get(),
          SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(AlbumArtLoaded(quint64,QImage)));
}

SearchProvider::ResultList LibrarySearchProvider::Search(int id, const QString& query) {
  QueryOptions options;
  options.set_filter(query);

  LibraryQuery q(options);
  q.SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);

  if (!backend_->ExecQuery(&q)) {
    return ResultList();
  }

  const QStringList tokens = TokenizeQuery(query);

  QMultiMap<QString, Song> albums;
  QSet<QString> albums_with_non_track_matches;

  ResultList ret;

  while (q.Next()) {
    Song song;
    song.InitFromQuery(q, true);

    QString album_key = song.album();
    if (song.is_compilation() && !song.albumartist().isEmpty()) {
      album_key.prepend(song.albumartist() + " - ");
    } else if (!song.is_compilation()) {
      album_key.prepend(song.artist());
    }

    Result::MatchQuality quality = MatchQuality(tokens, song.title());

    if (quality != Result::Quality_None) {
      // If the query matched in the song title then we're interested in this
      // as an individual song.
      Result result(this);
      result.type_ = Result::Type_Track;
      result.metadata_ = song;
      result.match_quality_ = quality;
      ret << result;
    } else {
      // Otherwise we record this as being an interesting album.
      albums_with_non_track_matches.insert(album_key);
    }

    albums.insertMulti(album_key, song);
  }

  // Add any albums that contained least one song that wasn't matched on the
  // song title.
  foreach (const QString& key, albums_with_non_track_matches) {
    Result result(this);
    result.type_ = Result::Type_Album;
    result.metadata_ = albums.value(key);
    result.album_size_ = albums.count(key);
    result.match_quality_ =
        qMin(
          MatchQuality(tokens, result.metadata_.albumartist()),
          qMin(MatchQuality(tokens, result.metadata_.artist()),
               MatchQuality(tokens, result.metadata_.album())));
    ret << result;
  }

  return ret;
}

void LibrarySearchProvider::LoadArtAsync(int id, const Result& result) {
  quint64 loader_id = cover_loader_->Worker()->LoadImageAsync(result.metadata_);
  cover_loader_tasks_[loader_id] = id;
}

void LibrarySearchProvider::AlbumArtLoaded(quint64 id, const QImage& image) {
  if (!cover_loader_tasks_.contains(id))
    return;
  int orig_id = cover_loader_tasks_.take(id);

  emit ArtLoaded(orig_id, image);
}

void LibrarySearchProvider::LoadTracksAsync(int id, const Result& result) {
  SongList ret;

  switch (result.type_) {
  case Result::Type_Track:
    // This is really easy - we just emit the track again.
    ret << result.metadata_;
    break;

  case Result::Type_Album:  {
    // Find all the songs in this album.
    LibraryQuery query;
    query.SetOrderBy("track");
    query.SetColumnSpec("ROWID, " + Song::kColumnSpec);
    query.AddCompilationRequirement(result.metadata_.is_compilation());
    query.AddWhere("album", result.metadata_.album());

    if (!result.metadata_.is_compilation())
      query.AddWhere("artist", result.metadata_.artist());

    if (!backend_->ExecQuery(&query)) {
      break;
    }

    while (query.Next()) {
      Song song;
      song.InitFromQuery(query, true);
      ret << song;
    }
  }
  }

  SongMimeData* mime_data = new SongMimeData;
  mime_data->backend = backend_;
  mime_data->songs = ret;

  emit TracksLoaded(id, mime_data);
}
