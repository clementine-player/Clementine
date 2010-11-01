/* This file is part of Clementine.

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

#include "playlistfilter.h"

#include <QtDebug>

PlaylistFilter::PlaylistFilter(QObject *parent)
  : QSortFilterProxyModel(parent),
    query_hash_(0)
{
  setDynamicSortFilter(true);

  column_names_["title"] = Playlist::Column_Title;
  column_names_["name"] = Playlist::Column_Title;
  column_names_["artist"] = Playlist::Column_Artist;
  column_names_["album"] = Playlist::Column_Album;
  column_names_["albumartist"] = Playlist::Column_AlbumArtist;
  column_names_["composer"] = Playlist::Column_Composer;
  column_names_["length"] = Playlist::Column_Length;
  column_names_["track"] = Playlist::Column_Track;
  column_names_["disc"] = Playlist::Column_Disc;
  column_names_["year"] = Playlist::Column_Year;
  column_names_["genre"] = Playlist::Column_Genre;
  column_names_["score"] = Playlist::Column_Score;

  exact_columns_ << Playlist::Column_Length
                 << Playlist::Column_Track
                 << Playlist::Column_Disc
                 << Playlist::Column_Year;
}

void PlaylistFilter::sort(int column, Qt::SortOrder order) {
  // Pass this through to the Playlist, it does sorting itself
  sourceModel()->sort(column, order);
}

bool PlaylistFilter::filterAcceptsRow(int row, const QModelIndex &parent) const {
  QString filter = filterRegExp().pattern().toLower();

  uint hash = qHash(filter);
  if (hash != query_hash_) {
    // Parse the query
    query_cache_.clear();

    QStringList sections = filter.simplified().split(' ');
    foreach (const QString& section, sections) {
      QString key = section.section(':', 0, 0).toLower();
      if (section.contains(':') && column_names_.contains(key)) {
        // Specific column
        query_cache_ << SearchTerm(
            section.section(':', 1, -1).toLower(),
            column_names_[key],
            exact_columns_.contains(column_names_[key]));
      } else {
        query_cache_ << SearchTerm(section);
      }
    }

    query_hash_ = hash;
  }

  // Test the row
  QString all_columns;

  foreach (const SearchTerm& term, query_cache_) {
    if (term.column_ != -1) {
      // Specific column
      QModelIndex index(sourceModel()->index(row, term.column_, parent));
      QString value(index.data().toString().toLower());

      if (term.exact_ && value != term.value_)
        return false;
      else if (!term.exact_ && !value.contains(term.value_))
        return false;
    } else {
      // All columns
      if (all_columns.isNull()) {
        // Cache the concatenated value of all columns
        foreach (int column, column_names_.values()) {
          QModelIndex index(sourceModel()->index(row, column, parent));
          all_columns += index.data().toString().toLower() + ' ';
        }
      }

      if (!all_columns.contains(term.value_))
        return false;
    }
  }
  return true;
}
