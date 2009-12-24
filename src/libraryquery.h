#ifndef LIBRARYQUERY_H
#define LIBRARYQUERY_H

#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QStringList>
#include <QVariantList>

struct QueryOptions {
  QueryOptions() : max_age(-1) {}

  QString filter;
  int max_age;
};

class LibraryQuery {
 public:
  LibraryQuery();
  LibraryQuery(const QueryOptions& options);

  void SetColumnSpec(const QString& spec) { column_spec_ = spec; }
  void AddWhere(const QString& column, const QVariant& value);

  QSqlQuery Query(QSqlDatabase db) const;

 private:
  QString column_spec_;
  QStringList where_clauses_;
  QVariantList bound_values_;
};

#endif // LIBRARYQUERY_H
