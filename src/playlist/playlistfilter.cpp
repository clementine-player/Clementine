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

#include "playlistfilter.h"
#include "playlistfilterparser.h"

#include <QtDebug>

PlaylistFilter::PlaylistFilter(QObject* parent)
    : QSortFilterProxyModel(parent),
      filter_tree_(new NopFilter),
      query_hash_(0) {
  setDynamicSortFilter(true);

  column_names_["title"] = Playlist::Column_Title;
  column_names_["name"] = Playlist::Column_Title;
  column_names_["artist"] = Playlist::Column_Artist;
  column_names_["album"] = Playlist::Column_Album;
  column_names_["albumartist"] = Playlist::Column_AlbumArtist;
  column_names_["composer"] = Playlist::Column_Composer;
  column_names_["performer"] = Playlist::Column_Performer;
  column_names_["grouping"] = Playlist::Column_Grouping;
  column_names_["length"] = Playlist::Column_Length;
  column_names_["track"] = Playlist::Column_Track;
  column_names_["disc"] = Playlist::Column_Disc;
  column_names_["year"] = Playlist::Column_Year;
  column_names_["originalyear"] = Playlist::Column_OriginalYear;
  column_names_["genre"] = Playlist::Column_Genre;
  column_names_["score"] = Playlist::Column_Score;
  column_names_["comment"] = Playlist::Column_Comment;
  column_names_["bpm"] = Playlist::Column_BPM;
  column_names_["bitrate"] = Playlist::Column_Bitrate;
  column_names_["filename"] = Playlist::Column_Filename;
  column_names_["rating"] = Playlist::Column_Rating;

  numerical_columns_ << Playlist::Column_Length << Playlist::Column_Track
                     << Playlist::Column_Disc << Playlist::Column_Year
                     << Playlist::Column_OriginalYear << Playlist::Column_Score
                     << Playlist::Column_BPM << Playlist::Column_Bitrate
                     << Playlist::Column_Rating;
}

PlaylistFilter::~PlaylistFilter() {}

void PlaylistFilter::sort(int column, Qt::SortOrder order) {
  // Pass this through to the Playlist, it does sorting itself
  sourceModel()->sort(column, order);
}

bool PlaylistFilter::filterAcceptsRow(int row,
                                      const QModelIndex& parent) const {
  QString filter = filterRegExp().pattern();

  uint hash = qHash(filter);
  if (hash != query_hash_) {
    // Parse the query
    FilterParser p(filter, column_names_, numerical_columns_);
    filter_tree_.reset(p.parse());

    query_hash_ = hash;
  }

  // Test the row
  return filter_tree_->accept(row, parent, sourceModel());
}
