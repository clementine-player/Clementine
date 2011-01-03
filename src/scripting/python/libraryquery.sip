class QueryOptions {

%TypeHeaderCode
#include "library/libraryquery.h"
%End

public:
  QueryOptions();

  bool Matches(const Song& song) const;

  QString filter;
  int max_age;
};

class LibraryQuery {

%TypeHeaderCode
#include "library/libraryquery.h"
%End

public:
  LibraryQuery(const QueryOptions& options = QueryOptions());

  void SetColumnSpec(const QString& spec);
  void SetOrderBy(const QString& order_by);
  void AddWhere(const QString& column, const QVariant& value, const QString& op = "=");
  void AddCompilationRequirement(bool compilation);
  void SetLimit(int limit);

  bool Next();
  QVariant Value(int column) const;
};
