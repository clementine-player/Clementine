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

#ifndef LIBRARYQUERY_H
#define LIBRARYQUERY_H

#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QStringList>
#include <QVariantList>

class Song;
class LibraryBackend;

// This structure let's you customize behaviour of any LibraryQuery.
struct QueryOptions {
  // Modes of LibraryQuery:
  // - use the all songs table
  // - use the duplicated songs view; by duplicated we mean those songs
  //   for which the (artist, album, title) tuple is found more than once
  //   in the songs table
  // - use the untagged songs view; by untagged we mean those for which
  //   at least one of the (artist, album, title) tags is empty
  // Please note that additional filtering based on fts table (the filter
  // attribute) won't work in Duplicates and Untagged modes.
  enum QueryMode { QueryMode_All, QueryMode_Duplicates, QueryMode_Untagged };

  QueryOptions();

  bool Matches(const Song& song) const;

  QString filter() const { return filter_; }
  void set_filter(const QString& filter) {
    this->filter_ = filter;
    this->query_mode_ = QueryMode_All;
  }

  int max_age() const { return max_age_; }
  void set_max_age(int max_age) { this->max_age_ = max_age; }

  QueryMode query_mode() const { return query_mode_; }
  void set_query_mode(QueryMode query_mode) {
    this->query_mode_ = query_mode;
    this->filter_ = QString();
  }

 private:
  QString filter_;
  int max_age_;
  QueryMode query_mode_;
};

class LibraryQuery {
 public:
  LibraryQuery(const QueryOptions& options = QueryOptions());

  // Sets contents of SELECT clause on the query (list of columns to get).
  void SetColumnSpec(const QString& spec) { column_spec_ = spec; }
  // Sets an ORDER BY clause on the query.
  void SetOrderBy(const QString& order_by) { order_by_ = order_by; }

  // Adds a fragment of WHERE clause. When executed, this Query will connect all
  // the fragments with AND operator.
  // Please note that IN operator expects a QStringList as value.
  void AddWhere(const QString& column, const QVariant& value,
                const QString& op = "=");

  void AddCompilationRequirement(bool compilation);
  void SetLimit(int limit) { limit_ = limit; }
  void SetIncludeUnavailable(bool include_unavailable) {
    include_unavailable_ = include_unavailable;
  }

  QSqlQuery Exec(QSqlDatabase db, const QString& songs_table,
                 const QString& fts_table);
  bool Next();
  QVariant Value(int column) const;

  operator const QSqlQuery&() const { return query_; }

 private:
  QString GetInnerQuery();

  bool include_unavailable_;
  bool join_with_fts_;
  QString column_spec_;
  QString order_by_;
  QStringList where_clauses_;
  QVariantList bound_values_;
  int limit_;
  bool duplicates_only_;

  QSqlQuery query_;
};

#endif  // LIBRARYQUERY_H
