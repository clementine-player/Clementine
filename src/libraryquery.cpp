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

#include "libraryquery.h"
#include "song.h"

#include <QtDebug>
#include <QDateTime>
#include <QSqlError>

QueryOptions::QueryOptions()
  : max_age(-1)
{
  group_by[0] = GroupBy_Artist;
  group_by[1] = GroupBy_Album;
  group_by[2] = GroupBy_None;
}


LibraryQuery::LibraryQuery()
{
}

LibraryQuery::LibraryQuery(const QueryOptions& options)
{
  if (!options.filter.isEmpty()) {
    where_clauses_ << "("
        "artist LIKE ? OR "
        "album LIKE ? OR "
        "title LIKE ?)";
    bound_values_ << "%" + options.filter + "%";
    bound_values_ << "%" + options.filter + "%";
    bound_values_ << "%" + options.filter + "%";
  }

  if (options.max_age != -1) {
    int cutoff = QDateTime::currentDateTime().toTime_t() - options.max_age;

    where_clauses_ << "ctime > ?";
    bound_values_ << cutoff;
  }
}

void LibraryQuery::AddWhere(const QString& column, const QVariant& value) {
  // Do integers inline - sqlite seems to get confused when you pass integers
  // to bound parameters

  if (value.type() == QVariant::Int)
    where_clauses_ << QString("%1 = %2").arg(column, value.toString());
  else {
    where_clauses_ << QString("%1 = ?").arg(column);
    bound_values_ << value;
  }
}

void LibraryQuery::AddWhereLike(const QString& column, const QVariant& value) {
  where_clauses_ << QString("%1 LIKE ?").arg(column);
  bound_values_ << value;
}

void LibraryQuery::AddCompilationRequirement(bool compilation) {
  where_clauses_ << QString("effective_compilation = %1").arg(compilation ? 1 : 0);
}

QSqlError LibraryQuery::Exec(QSqlDatabase db) {
  QString sql = QString("SELECT %1 FROM songs").arg(column_spec_);

  if (!where_clauses_.isEmpty())
    sql += " WHERE " + where_clauses_.join(" AND ");

  if (!order_by_.isEmpty())
    sql += " ORDER BY " + order_by_;

  query_ = QSqlQuery(sql, db);

  // Bind values
  foreach (const QVariant& value, bound_values_) {
    query_.addBindValue(value);
  }

  query_.exec();
  return query_.lastError();
}

bool LibraryQuery::Next() {
  return query_.next();
}

QVariant LibraryQuery::Value(int column) const {
  return query_.value(column);
}

bool QueryOptions::Matches(const Song& song) const {
  if (max_age != -1) {
    const uint cutoff = QDateTime::currentDateTime().toTime_t() - max_age;
    if (song.ctime() <= cutoff)
      return false;
  }

  if (!filter.isNull()) {
    return song.artist().contains(filter, Qt::CaseInsensitive) ||
           song.album().contains(filter, Qt::CaseInsensitive) ||
           song.title().contains(filter, Qt::CaseInsensitive);
  }

  return true;
}
