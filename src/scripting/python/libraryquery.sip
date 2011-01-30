class QueryOptions {

%TypeHeaderCode
#include "library/libraryquery.h"
%End

public:
  QueryOptions();

  bool Matches(const Song& song) const;

  QString filter;
  int max_age;
  bool duplicates_only;
};

class LibraryQuery {

%TypeHeaderCode
#include "library/libraryquery.h"
%End

public:
  LibraryQuery(); // Workaround SIP Mercurial 3e647ed0f2a2
  LibraryQuery(const QueryOptions& options);

  void SetColumnSpec(const QString& spec);
  void SetOrderBy(const QString& order_by);

  void AddWhere(const QString& column, const QVariant& value, const QString& op = "=");

  void AddCompilationRequirement(bool compilation);
  void SetLimit(int limit);

  bool Next();
  QVariant Value(int column) const;
};
