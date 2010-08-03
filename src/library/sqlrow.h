#ifndef SQLROW_H
#define SQLROW_H

#include <QList>
#include <QVariant>

class QSqlQuery;

class SqlRow {
 public:
  // WARNING: Implicit construction from QSqlQuery and LibraryQuery.
  SqlRow(const QSqlQuery& query);

  QVariant value(int i) const { return columns_[i]; }

 private:
  SqlRow();

  QList<QVariant> columns_;
};

#endif
