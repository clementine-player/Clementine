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
#include "sqlrow.h"

#include <QSqlQuery>
#include <QSqlRecord>

SqlRow::SqlRow(const QSqlQuery& query) { Init(query); }

SqlRow::SqlRow(const LibraryQuery& query) { Init(query); }

void SqlRow::Init(const QSqlQuery& query) {
  int rows = query.record().count();
  for (int i = 0; i < rows; ++i) {
    columns_ << query.value(i);
  }
}
