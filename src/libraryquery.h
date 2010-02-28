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
  void AddCompilationRequirement(bool compilation);

  QSqlQuery Query(QSqlDatabase db) const;

 private:
  QString column_spec_;
  QString order_by_;
  QStringList where_clauses_;
  QVariantList bound_values_;
};

#endif // LIBRARYQUERY_H
