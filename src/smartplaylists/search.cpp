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

#include "search.h"
#include "core/logging.h"
#include "core/song.h"

#include <QStringList>

namespace smart_playlists {

Search::Search() { Reset(); }

Search::Search(SearchType type, TermList terms, SortType sort_type,
               SearchTerm::Field sort_field, int limit)
    : search_type_(type),
      terms_(terms),
      sort_type_(sort_type),
      sort_field_(sort_field),
      limit_(limit),
      first_item_(0) {}

void Search::Reset() {
  search_type_ = Type_And;
  terms_.clear();
  sort_type_ = Sort_Random;
  sort_field_ = SearchTerm::Field_Title;
  limit_ = -1;
  first_item_ = 0;
}

QString Search::ToSql(const QString& songs_table) const {
  QString sql = "SELECT ROWID," + Song::kColumnSpec + " FROM " + songs_table;

  // Add search terms
  QStringList where_clauses;
  QStringList term_where_clauses;
  foreach(const SearchTerm & term, terms_) {
    term_where_clauses << term.ToSql();
  }

  if (!terms_.isEmpty() && search_type_ != Type_All) {
    QString boolean_op = search_type_ == Type_And ? " AND " : " OR ";
    where_clauses << "(" + term_where_clauses.join(boolean_op) + ")";
  }

  // Restrict the IDs of songs if we're making a dynamic playlist
  if (!id_not_in_.isEmpty()) {
    QString numbers;
    foreach(int id, id_not_in_) {
      numbers += (numbers.isEmpty() ? "" : ",") + QString::number(id);
    }
    where_clauses << "(ROWID NOT IN (" + numbers + "))";
  }

  // We never want to include songs that have been deleted, but are still kept
  // in the database in case the directory containing them has just been
  // unmounted.
  where_clauses << "unavailable = 0";

  if (!where_clauses.isEmpty()) {
    sql += " WHERE " + where_clauses.join(" AND ");
  }

  // Add sort by
  if (sort_type_ == Sort_Random) {
    sql += " ORDER BY random()";
  } else {
    sql += " ORDER BY " + SearchTerm::FieldColumnName(sort_field_) +
           (sort_type_ == Sort_FieldAsc ? " ASC" : " DESC");
  }

  // Add limit
  if (first_item_) {
    sql += QString(" LIMIT %1 OFFSET %2").arg(limit_).arg(first_item_);
  } else if (limit_ != -1) {
    sql += " LIMIT " + QString::number(limit_);
  }
  qLog(Debug) << sql;

  return sql;
}

bool Search::is_valid() const {
  if (search_type_ == Type_All) return true;
  return !terms_.isEmpty();
}

bool Search::operator==(const Search& other) const {
  return search_type_ == other.search_type_ && terms_ == other.terms_ &&
         sort_type_ == other.sort_type_ && sort_field_ == other.sort_field_ &&
         limit_ == other.limit_;
}

}  // namespace

QDataStream& operator<<(QDataStream& s, const smart_playlists::Search& search) {
  s << search.terms_;
  s << quint8(search.sort_type_);
  s << quint8(search.sort_field_);
  s << qint32(search.limit_);
  s << quint8(search.search_type_);
  return s;
}

QDataStream& operator>>(QDataStream& s, smart_playlists::Search& search) {
  quint8 sort_type, sort_field, search_type;
  qint32 limit;

  s >> search.terms_ >> sort_type >> sort_field >> limit >> search_type;
  search.sort_type_ = smart_playlists::Search::SortType(sort_type);
  search.sort_field_ = smart_playlists::SearchTerm::Field(sort_field);
  search.limit_ = limit;
  search.search_type_ = smart_playlists::Search::SearchType(search_type);

  return s;
}
