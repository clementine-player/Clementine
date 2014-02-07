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

#include "libraryquery.h"
#include "core/song.h"

#include <QtDebug>
#include <QDateTime>
#include <QSqlError>

QueryOptions::QueryOptions() : max_age_(-1), query_mode_(QueryMode_All) {}

LibraryQuery::LibraryQuery(const QueryOptions& options)
    : include_unavailable_(false), join_with_fts_(false), limit_(-1) {
  if (!options.filter().isEmpty()) {
    // We need to munge the filter text a little bit to get it to work as
    // expected with sqlite's FTS3:
    //  1) Append * to all tokens.
    //  2) Prefix "fts" to column names.
    //  3) Remove colons which don't correspond to column names.

    // Split on whitespace
    QStringList tokens(
        options.filter().split(QRegExp("\\s+"), QString::SkipEmptyParts));
    QString query;
    foreach(QString token, tokens) {
      token.remove('(');
      token.remove(')');
      token.remove('"');
      token.replace('-', ' ');

      if (token.contains(':')) {
        // Only prefix fts if the token is a valid column name.
        if (Song::kFtsColumns.contains("fts" + token.section(':', 0, 0),
                                       Qt::CaseInsensitive)) {
          // Account for multiple colons.
          QString columntoken =
              token.section(':', 0, 0, QString::SectionIncludeTrailingSep);
          QString subtoken = token.section(':', 1, -1);
          subtoken.replace(":", " ");
          subtoken = subtoken.trimmed();
          query += "fts" + columntoken + subtoken + "* ";
        } else {
          token.replace(":", " ");
          token = token.trimmed();
          query += token + "* ";
        }
      } else {
        query += token + "* ";
      }
    }

    where_clauses_ << "fts.%fts_table_noprefix MATCH ?";
    bound_values_ << query;
    join_with_fts_ = true;
  }

  if (options.max_age() != -1) {
    int cutoff = QDateTime::currentDateTime().toTime_t() - options.max_age();

    where_clauses_ << "ctime > ?";
    bound_values_ << cutoff;
  }

  // TODO: currently you cannot use any QueryMode other than All and fts at the
  // same time.
  // joining songs, duplicated_songs and songs_fts all together takes a huge
  // amount of
  // time. the query takes about 20 seconds on my machine then. why?
  // untagged mode could work with additional filtering but I'm disabling it
  // just to be
  // consistent - this way filtering is available only in the All mode.
  // remember though that when you fix the Duplicates + FTS cooperation, enable
  // the
  // filtering in both Duplicates and Untagged modes.
  duplicates_only_ = options.query_mode() == QueryOptions::QueryMode_Duplicates;

  if (options.query_mode() == QueryOptions::QueryMode_Untagged) {
    where_clauses_ << "(artist = '' OR album = '' OR title ='')";
  }
}

QString LibraryQuery::GetInnerQuery() {
  return duplicates_only_
             ? QString(
                   " INNER JOIN (select * from duplicated_songs) dsongs        "
                   "ON (%songs_table.artist = dsongs.dup_artist       "
                   "AND %songs_table.album = dsongs.dup_album     "
                   "AND %songs_table.title = dsongs.dup_title)    ")
             : QString();
}

void LibraryQuery::AddWhere(const QString& column, const QVariant& value,
                            const QString& op) {
  // ignore 'literal' for IN
  if (!op.compare("IN", Qt::CaseInsensitive)) {
    QStringList final;
    foreach(const QString & single_value, value.toStringList()) {
      final.append("?");
      bound_values_ << single_value;
    }

    where_clauses_ << QString("%1 IN (" + final.join(",") + ")").arg(column);
  } else {
    // Do integers inline - sqlite seems to get confused when you pass integers
    // to bound parameters
    if (value.type() == QVariant::Int) {
      where_clauses_ << QString("%1 %2 %3").arg(column, op, value.toString());
    } else {
      where_clauses_ << QString("%1 %2 ?").arg(column, op);
      bound_values_ << value;
    }
  }
}

void LibraryQuery::AddCompilationRequirement(bool compilation) {
  where_clauses_ << QString("effective_compilation = %1")
                        .arg(compilation ? 1 : 0);
}

QSqlQuery LibraryQuery::Exec(QSqlDatabase db, const QString& songs_table,
                             const QString& fts_table) {
  QString sql;

  if (join_with_fts_) {
    sql = QString(
              "SELECT %1 FROM %2 INNER JOIN %3 AS fts ON %2.ROWID = fts.ROWID")
              .arg(column_spec_, songs_table, fts_table);
  } else {
    sql = QString("SELECT %1 FROM %2 %3")
              .arg(column_spec_, songs_table, GetInnerQuery());
  }

  QStringList where_clauses(where_clauses_);
  if (!include_unavailable_) {
    where_clauses << "unavailable = 0";
  }

  if (!where_clauses.isEmpty()) sql += " WHERE " + where_clauses.join(" AND ");

  if (!order_by_.isEmpty()) sql += " ORDER BY " + order_by_;

  if (limit_ != -1) sql += " LIMIT " + QString::number(limit_);

  sql.replace("%songs_table", songs_table);
  sql.replace("%fts_table_noprefix", fts_table.section('.', -1, -1));
  sql.replace("%fts_table", fts_table);

  query_ = QSqlQuery(sql, db);

  // Bind values
  foreach(const QVariant & value, bound_values_) { query_.addBindValue(value); }

  query_.exec();
  return query_;
}

bool LibraryQuery::Next() { return query_.next(); }

QVariant LibraryQuery::Value(int column) const { return query_.value(column); }

bool QueryOptions::Matches(const Song& song) const {
  if (max_age_ != -1) {
    const uint cutoff = QDateTime::currentDateTime().toTime_t() - max_age_;
    if (song.ctime() <= cutoff) return false;
  }

  if (!filter_.isNull()) {
    return song.artist().contains(filter_, Qt::CaseInsensitive) ||
           song.album().contains(filter_, Qt::CaseInsensitive) ||
           song.title().contains(filter_, Qt::CaseInsensitive);
  }

  return true;
}
