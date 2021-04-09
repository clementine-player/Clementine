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

#include <QStack>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

#include "core/logging.h"
#include "covers/albumcoverloader.h"
#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "library/sqlrow.h"
#include "playlist/songmimedata.h"

LibrarySearchProvider::LibrarySearchProvider(LibraryBackendInterface* backend,
                                             const QString& name,
                                             const QString& id,
                                             const QIcon& icon,
                                             bool enabled_by_default,
                                             Application* app, QObject* parent)
    : BlockingSearchProvider(app, parent), backend_(backend) {
  Hints hints =
      WantsSerialisedArtQueries | ArtIsInSongMetadata | CanGiveSuggestions;

  if (!enabled_by_default) {
    hints |= DisabledByDefault;
  }

  Init(name, id, icon, hints);
}

SearchProvider::ResultList LibrarySearchProvider::Search(int id,
                                                         const QString& query) {
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

MimeData* LibrarySearchProvider::LoadTracks(const ResultList& results) {
  MimeData* ret = SearchProvider::LoadTracks(results);
  static_cast<SongMimeData*>(ret)->backend = backend_;

  return ret;
}

QStringList LibrarySearchProvider::GetSuggestions(int count) {
  // We'd like to use order by random(), but that's O(n) in sqlite, so instead
  // get the largest ROWID and pick a couple of random numbers within that
  // range.

  LibraryQuery q;
  q.SetColumnSpec("ROWID");
  q.SetOrderBy("ROWID DESC");
  q.SetIncludeUnavailable(true);
  q.SetLimit(1);

  QStringList ret;

  if (!backend_->ExecQuery(&q) || !q.Next()) {
    return ret;
  }

  const int largest_rowid = q.Value(0).toInt();

  for (int attempt = 0; attempt < count * 5; ++attempt) {
    if (ret.count() >= count) {
      break;
    }

    LibraryQuery q;
    q.SetColumnSpec("artist, album");
    q.SetIncludeUnavailable(true);
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    q.AddWhere("ROWID", qrand() % largest_rowid);
#else
    q.AddWhere("ROWID", QRandomGenerator::global()->bounded(largest_rowid));
#endif
    q.SetLimit(1);

    if (!backend_->ExecQuery(&q) || !q.Next()) {
      continue;
    }

    const QString artist = q.Value(0).toString();
    const QString album = q.Value(1).toString();

    if (!artist.isEmpty() && !album.isEmpty())
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
      ret << ((qrand() % 2 == 0) ? artist : album);
#else
      ret << (QRandomGenerator::global()->bounded(2) == 0 ? artist : album);
#endif
    else if (!artist.isEmpty())
      ret << artist;
    else if (!album.isEmpty())
      ret << album;
  }

  return ret;
}
