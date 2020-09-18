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

#ifndef SMARTPLAYLISTSEARCH_H
#define SMARTPLAYLISTSEARCH_H

#include "generator.h"
#include "searchterm.h"

namespace smart_playlists {

class Search {
 public:
  typedef QList<SearchTerm> TermList;

  // These values are persisted, so add to the end of the enum only
  enum SearchType {
    Type_And = 0,
    Type_Or,
    Type_All,
  };

  // These values are persisted, so add to the end of the enum only
  enum SortType {
    Sort_Random = 0,
    Sort_FieldAsc,
    Sort_FieldDesc,
  };

  Search();
  Search(SearchType type, TermList terms, SortType sort_type,
         SearchTerm::Field sort_field, int limit = Generator::kDefaultLimit);

  bool is_valid() const;
  bool operator==(const Search& other) const;
  bool operator!=(const Search& other) const { return !(*this == other); }

  SearchType search_type_;
  TermList terms_;
  SortType sort_type_;
  SearchTerm::Field sort_field_;
  int limit_;

  // Not persisted, used to alter the behaviour of the query
  QList<int> id_not_in_;
  int first_item_;

  void Reset();
  QString ToSql(const QString& songs_table) const;
};

}  // namespace smart_playlists

QDataStream& operator<<(QDataStream& s, const smart_playlists::Search& search);
QDataStream& operator>>(QDataStream& s, smart_playlists::Search& search);

#endif  // SMARTPLAYLISTSEARCH_H
