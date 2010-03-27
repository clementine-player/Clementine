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

#ifndef LIBRARYQUERY_H
#define LIBRARYQUERY_H

#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QStringList>
#include <QVariantList>

class Song;

struct QueryOptions {
  QueryOptions() : max_age(-1) {}

  bool Matches(const Song& song) const;

  QString filter;
  int max_age;
};

class LibraryQuery {
 public:
  LibraryQuery();
  LibraryQuery(const QueryOptions& options);

  void SetColumnSpec(const QString& spec) { column_spec_ = spec; }
  void SetOrderBy(const QString& order_by) { order_by_ = order_by; }
  void AddWhere(const QString& column, const QVariant& value);
  void AddWhereLike(const QString& column, const QVariant& value);
  void AddCompilationRequirement(bool compilation);

  QSqlQuery Query(QSqlDatabase db) const;

 private:
  QString column_spec_;
  QString order_by_;
  QStringList where_clauses_;
  QVariantList bound_values_;
};

#endif // LIBRARYQUERY_H
