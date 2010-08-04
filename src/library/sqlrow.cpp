#include "libraryquery.h"
#include "sqlrow.h"

#include <QSqlQuery>
#include <QSqlRecord>

SqlRow::SqlRow(const QSqlQuery& query) {
  Init(query);
}

SqlRow::SqlRow(const LibraryQuery& query) {
  Init(query);
}

void SqlRow::Init(const QSqlQuery& query) {
  int rows = query.record().count();
  for (int i = 0; i < rows; ++i) {
    columns_ << query.value(i);
  }
}

