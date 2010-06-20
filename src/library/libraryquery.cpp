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
#include "core/song.h"

#include <QtDebug>
#include <QDateTime>
#include <QSqlError>

QueryOptions::QueryOptions()
  : max_age(-1)
{
}


LibraryQuery::LibraryQuery(const QueryOptions& options)
  : join_with_fts_(false),
    limit_(-1)
{
  if (!options.filter.isEmpty()) {
    // We need to munge the filter text a little bit to get it to work as
    // expected with sqlite's FTS3:
    //  1) Append * to all tokens.
    //  2) Prefix "fts" to column names.

    // Split on whitespace
    QStringList tokens(options.filter.split(QRegExp("\\s+")));
    QString query;
    foreach (const QString& token, tokens) {
      if (token.contains(':'))
        query += "fts" + token + "* ";
      else
        query += token + "* ";
    }

    where_clauses_ << "fts.%fts_table MATCH ?";
    bound_values_ << query;
    join_with_fts_ = true;
  }

  if (options.max_age != -1) {
    int cutoff = QDateTime::currentDateTime().toTime_t() - options.max_age;

    where_clauses_ << "ctime > ?";
    bound_values_ << cutoff;
  }
}

void LibraryQuery::AddWhere(const QString& column, const QVariant& value, const QString& op) {
  // Do integers inline - sqlite seems to get confused when you pass integers
  // to bound parameters

  if (value.type() == QVariant::Int)
    where_clauses_ << QString("%1 %2 %3").arg(column, op, value.toString());
  else {
    where_clauses_ << QString("%1 %2 ?").arg(column, op);
    bound_values_ << value;
  }
}

void LibraryQuery::AddCompilationRequirement(bool compilation) {
  where_clauses_ << QString("effective_compilation = %1").arg(compilation ? 1 : 0);
}

QSqlError LibraryQuery::Exec(QSqlDatabase db, const QString& songs_table,
                             const QString& fts_table) {
  QString sql;
  if (join_with_fts_) {
    sql = QString("SELECT %1 FROM %2 INNER JOIN %3 AS fts ON %2.ROWID = fts.ROWID")
          .arg(column_spec_, songs_table, fts_table);
  } else {
    sql = QString("SELECT %1 FROM %2")
          .arg(column_spec_, songs_table);
  }

  if (!where_clauses_.isEmpty())
    sql += " WHERE " + where_clauses_.join(" AND ");

  if (!order_by_.isEmpty())
    sql += " ORDER BY " + order_by_;

  if (limit_ != -1)
    sql += " LIMIT " + QString::number(limit_);

  sql.replace("%songs_table", songs_table);
  sql.replace("%fts_table", fts_table);
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
