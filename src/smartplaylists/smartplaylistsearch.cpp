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

#include "smartplaylistsearch.h"
#include "core/song.h"

#include <QStringList>

SmartPlaylistSearch::SmartPlaylistSearch() {
  Reset();
}

void SmartPlaylistSearch::Reset() {
  terms_.clear();
  sort_type_ = Sort_Random;
  sort_field_ = SmartPlaylistSearchTerm::Field_Title;
  limit_ = -1;
}

QString SmartPlaylistSearch::ToSql(const QString& songs_table) const {
  QString sql = "SELECT ROWID," + Song::kColumnSpec + " FROM " + songs_table;

  // Add search terms
  QStringList term_sql;
  foreach (const SmartPlaylistSearchTerm& term, terms_) {
    term_sql += term.ToSql();
  }
  if (!terms_.isEmpty()) {
    sql += " WHERE " + term_sql.join(" AND ");
  }

  // Add sort by
  if (sort_type_ == Sort_Random) {
    sql += " ORDER BY random()";
  } else {
    sql += " ORDER BY " + SmartPlaylistSearchTerm::FieldColumnName(sort_field_)
        + (sort_type_ == Sort_FieldAsc ? " ASC" : " DESC");
  }

  // Add limit
  if (limit_ != -1) {
    sql += " LIMIT " + QString::number(limit_);
  }

  return sql;
}

QDataStream& operator <<(QDataStream& s, const SmartPlaylistSearch& search) {
  s << search.terms_;
  s << quint8(search.sort_type_);
  s << quint8(search.sort_field_);
  s << qint32(search.limit_);
  return s;
}

QDataStream& operator >>(QDataStream& s, SmartPlaylistSearch& search) {
  quint8 sort_type, sort_field;
  qint32 limit;

  s >> search.terms_ >> sort_type >> sort_field >> limit;
  search.sort_type_ = SmartPlaylistSearch::SortType(sort_type);
  search.sort_field_ = SmartPlaylistSearchTerm::Field(sort_field);
  search.limit_ = limit;

  return s;
}
