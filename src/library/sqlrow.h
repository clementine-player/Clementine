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

  QVariant value(int i) const { return columns_[i]; }

 private:
  SqlRow();

  void Init(const QSqlQuery& query);

  QList<QVariant> columns_;
};

#endif
