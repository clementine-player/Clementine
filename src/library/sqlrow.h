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

#ifndef SQLROW_H
#define SQLROW_H

#include <QList>
#include <QVariant>

class QSqlQuery;

class LibraryQuery;

class SqlRow {
 public:
  // WARNING: Implicit construction from QSqlQuery and LibraryQuery.
  SqlRow(const QSqlQuery& query);
  SqlRow(const LibraryQuery& query);

  const QVariant& value(int i) const { return columns_[i]; }

 private:
  SqlRow();

  void Init(const QSqlQuery& query);

  QList<QVariant> columns_;
};

typedef QList<SqlRow> SqlRowList;

#endif
