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

#include <QStack>


LibrarySearchProvider::LibrarySearchProvider(LibraryBackendInterface* backend,
                                             const QString& name,
                                             const QString& id,
                                             const QIcon& icon,
                                             bool enabled_by_default,
                                             Application* app,
                                             QObject* parent)
  : BlockingSearchProvider(app, parent),
    backend_(backend)
{
  Hints hints = WantsSerialisedArtQueries | ArtIsInSongMetadata |
                CanGiveSuggestions;

  if (!enabled_by_default) {
    hints |= DisabledByDefault;
  }

  Init(name, id, icon, hints);
}

SearchProvider::ResultList LibrarySearchProvider::Search(int id, const QString& query) {
  QueryOptions options;
  options.set_filter(query);

  LibraryQuery q(options);
  q.SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);

  if (!backend_->ExecQuery(&q)) {
    return ResultList();
  }

  // Build the result list
  ResultList ret;
  while (q.Next()) {
    Result result(this);
    result.metadata_.InitFromQuery(q, true);
    ret << result;
  }

  return ret;
}

void LibrarySearchProvider::LoadTracksAsync(int id, const Result& result) {
  SongMimeData* mime_data = new SongMimeData;
  mime_data->backend = backend_;
  mime_data->songs = SongList() << result.metadata_;

  emit TracksLoaded(id, mime_data);
}

QString LibrarySearchProvider::GetSuggestion() {
  // We'd like to use order by random(), but that's O(n) in sqlite, so instead
  // get the largest ROWID and pick a couple of random numbers within that
  // range.

  LibraryQuery q;
  q.SetColumnSpec("ROWID");
  q.SetOrderBy("ROWID DESC");
  q.SetIncludeUnavailable(true);
  q.SetLimit(1);

  if (!backend_->ExecQuery(&q) || !q.Next()) {
    return QString();
  }

  const int largest_rowid = q.Value(0).toInt();

  for (int attempt=0 ; attempt<10 ; ++attempt) {
    LibraryQuery q;
    q.SetColumnSpec("artist, album");
    q.SetIncludeUnavailable(true);
    q.AddWhere("ROWID", qrand() % largest_rowid);
    q.SetLimit(1);

    if (!backend_->ExecQuery(&q) || !q.Next()) {
      continue;
    }

    const QString artist = q.Value(0).toString();
    const QString album  = q.Value(1).toString();

    if (!artist.isEmpty() && !album.isEmpty())
      return (qrand() % 2 == 0) ? artist : album;
    else if (!artist.isEmpty())
      return artist;
    else if (!album.isEmpty())
      return album;
  }

  return QString();
}
