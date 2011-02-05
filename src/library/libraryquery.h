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

struct QueryOptions {
  QueryOptions();

  bool Matches(const Song& song) const;

  // Please note that this attribute is mutually exclusive with the 'duplicates_only'
  // flag!
  QString filter;
  int max_age;
  // If true, the query will operate not on the whole songs table but only on those rows
  // which are duplicated. By duplication we mean a situation where two or more songs
  // with equal (artist, album, title) tuple exist (case sensitive).
  // Please note that this flag is mutually exclusive with the 'filter' attribute!
  bool duplicates_only;
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
  void AddWhere(const QString& column, const QVariant& value, const QString& op = "=");

  void AddCompilationRequirement(bool compilation);
  void SetLimit(int limit) { limit_ = limit; }

  QSqlQuery Exec(QSqlDatabase db, const QString& songs_table, const QString& fts_table);
  bool Next();
  QVariant Value(int column) const;

  operator const QSqlQuery& () const { return query_; }

 private:
  QString GetInnerQuery();

  bool join_with_fts_;
  QString column_spec_;
  QString order_by_;
  QStringList where_clauses_;
  QVariantList bound_values_;
  int limit_;
  bool duplicates_only_;

  QSqlQuery query_;
};

#endif // LIBRARYQUERY_H
